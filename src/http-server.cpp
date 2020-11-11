#include "http-server.hpp"

#include "http/client_http.hpp"
#include "http/server_http.hpp"
#include "http/client_https.hpp"
#include "http/server_https.hpp"
#include "clib/global_timer.hpp"
#include "clib/observation_module.hpp"

#define BOOST_SPIRIT_THREADSAFE
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include <algorithm>
#include <boost/filesystem.hpp>
#include <boost/random.hpp>
#include <sstream>
#include <fstream>
#include <atomic>
#include <regex>
#include <vector>
#include <future>
#include <chrono>
#ifdef HAVE_OPENSSL
#include "crypto.hpp"
#endif

extern std::atomic_ushort load_progress;

const std::string bad_input_format  = "Bad format, json is expected";
const std::string bad_input_members = "Bad format, members are missing";
const std::string bad_input_interval = "Bad interval in seconds, use int";
const std::string bad_observable_handle  = "Bad handle, no observable found";
const std::string bad_input_param_location = "Bad param location, use: path.to.param";
const std::string bad_input_device_location = "Bad URL, use: proto://address[:port]/[path]";

std::regex device_location_regex(R"(^(https?|coaps?):\/\/((?:[A-Za-z0-9-]+\.)+[A-Za-z0-9-]+(?::[0-9]+)?)(\/.*)$)");
std::regex param_location_regex(R"(^(?:[A-Za-z0-9_-]+\.)*[A-Za-z0-9_-]+$)");

using HttpsServer = SimpleWeb::Server<SimpleWeb::HTTPS>;
using HttpServer = SimpleWeb::Server<SimpleWeb::HTTP>;

using HttpsClient = SimpleWeb::Client<SimpleWeb::HTTPS>;
using HttpClient = SimpleWeb::Client<SimpleWeb::HTTP>;

typedef std::chrono::high_resolution_clock Clock;

extern int send_coap_request(const std::string & url, const std::string & path, const std::string & port);

void send_json_response(std::shared_ptr<HttpServer::Response> response, boost::property_tree::ptree root) {
    std::ostringstream reply;
    boost::property_tree::write_json(reply, root);
    *response << "HTTP/1.1 200 OK\r\nContent-Length: " << reply.str().length() << "\r\n\r\n" << reply.str();
}

boost::property_tree::ptree observable2json(const Observable & observable, size_t handle) {
    boost::property_tree::ptree root;
    root.put("name", observable.device_name);
    root.put("param", observable.param_name);
    root.put("handle", handle);

    boost::property_tree::ptree values;
    for (const auto & entry : observable.getValues()) {
        boost::property_tree::ptree cell;
        auto in_time_t = std::chrono::system_clock::to_time_t(entry.first);
        cell.put("date", std::put_time(std::localtime(&in_time_t), "%Y-%m-%dT%H:%M:%SZ")); //2015-03-25T12:00:00Z
        cell.put("value", entry.second);
        values.push_back(std::make_pair("", cell));
    }
    root.add_child("values", values);
    return root;
}

boost::property_tree::ptree http_response2json(std::string device_location, std::string device_path) {
    boost::property_tree::ptree root;
    HttpClient client(device_location);
    client.request("GET", device_path, {},
                   [&root](std::shared_ptr<HttpClient::Response> response, const SimpleWeb::error_code &ec) {
                       if (!ec) {
                           try { boost::property_tree::json_parser::read_json(response->content, root); }
                           catch (...) {}
                       }
                   });
    client.io_service->run();
    return root;
}

boost::property_tree::ptree https_response2json(std::string device_location, std::string device_path) {
    boost::property_tree::ptree root;
    HttpsClient client(device_location);
    client.request("GET", device_path, {},
                   [&root](std::shared_ptr<HttpsClient::Response> response, const SimpleWeb::error_code &ec) {
                       if (!ec) {
                           try { boost::property_tree::json_parser::read_json(response->content, root); }
                           catch (...) {}
                       }
                   });
    client.io_service->run();
    return root;
}

void start_http_server() {
    // HTTP-server at port 80 using 1 thread
    // Unless you do more heavy non-threaded processing in the resources,
    HttpServer server;
    server.config.port = 80;
    
    auto start_time = std::chrono::system_clock::now();
    server.resource["^/get-uptime$"]["GET"] = [&start_time](std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {
        boost::property_tree::ptree root;
        root.put("uptime", std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - start_time).count());
        send_json_response(response, root);
    };

    server.resource["^/add-new-observable$"]["POST"] = [](std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {
        boost::property_tree::ptree root;

        Observable::Protocol proto = Observable::Protocol::ProtocolHTTPS;
        std::string host, path, param_location, device_name, param_name;
        size_t interval;

        try { // dispatch request
            try { boost::property_tree::json_parser::read_json(request->content, root); } catch (...) {
                response->write(SimpleWeb::StatusCode::client_error_bad_request, bad_input_format);
                return;
            }
            std::smatch device_location_details;
            auto device_location = root.get<std::string>("device-location");
            if (!std::regex_match(device_location, device_location_details, device_location_regex)) {
                response->write(SimpleWeb::StatusCode::client_error_bad_request, bad_input_device_location);
                return;
            }
            if (std::string("http").compare(device_location_details[1]) == 0)
                proto = Observable::Protocol::ProtocolHTTP;
            else if (std::string("coap").compare(device_location_details[1]) == 0)
                proto = Observable::Protocol::ProtocolCOAP;
            else if (std::string("https").compare(device_location_details[1]) == 0)
                proto = Observable::Protocol::ProtocolHTTPS;
            else if (std::string("coaps").compare(device_location_details[1]) == 0)
                proto = Observable::Protocol::ProtocolCOAPS;
            host  = device_location_details[2];
            path  = device_location_details[3];
            param_location = root.get<std::string>("param-location");
            if (!std::regex_match(param_location, param_location_regex)) {
                response->write(SimpleWeb::StatusCode::client_error_bad_request, bad_input_param_location);
                return;
            }
            device_name = root.get<std::string>("device-name");
            param_name = root.get<std::string>("param-name");
            try { interval = root.get<size_t>("interval"); } catch (...) {
                response->write(SimpleWeb::StatusCode::client_error_bad_request, bad_input_interval);
                return;
            }
        } catch(...) {
            response->write(SimpleWeb::StatusCode::client_error_bad_request, bad_input_members);
            return;
        }

        // Everything is fine so create observable and associate corresponding callback
        boost::lock_guard<boost::mutex> lock(getObservables().lock);
        const auto handle = getObservables().create();
        getObservables().get(handle).param_location = param_location;
        getObservables().get(handle).device_name = device_name;
        getObservables().get(handle).param_name = param_name;
        getObservables().get(handle).interval = interval;
        getObservables().get(handle).protocol = proto;
        getObservables().get(handle).hostname = host;
        getObservables().get(handle).path = path;

        static std::unordered_map<size_t, Simple::Signal<void()>::SlotHandle> observable_callbacks;
        observable_callbacks[handle] = getTimer().connect([handle] () {
            try {
                boost::lock_guard<boost::mutex> lock(getObservables().lock);
                auto & observable = getObservables().get(handle);
                auto secs = std::chrono::duration_cast<std::chrono::seconds>(
                        std::chrono::system_clock::now() - observable.latest_observation).count();
                if (secs >= observable.interval) {
                    double value = 0;
                    try {
                        switch(observable.protocol) {
                        case Observable::Protocol::ProtocolHTTP:
                            value = http_response2json(observable.hostname, observable.path).get<double>(
                                    observable.param_location);
                            break;
                        case Observable::Protocol::ProtocolHTTPS:
                            value = https_response2json(observable.hostname, observable.path).get<double>(
                                    observable.param_location);
                            break;
                        }
                    } catch (...) {}
                    observable.addValue(value);
                    observable.latest_observation = std::chrono::system_clock::now();
                }
            } catch (...) {
                getTimer().disconnect(observable_callbacks[handle]);
                observable_callbacks.erase(handle);
            }
        });

        boost::property_tree::ptree results;
        results.put("result", "success");
        results.put("handle", handle);
        send_json_response(response, results);
    };

    server.resource["^/update-observable/([0-9]+)$"]["POST"] = [](std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {
        const size_t handle = std::stoi(request->path_match[1].str());
        std::string device_name, param_name, param_location;
        boost::property_tree::ptree root;

        try { // dispatch request
            try { boost::property_tree::json_parser::read_json(request->content, root); } catch (...) {
                response->write(SimpleWeb::StatusCode::client_error_bad_request, bad_input_format);
                return;
            }
            param_location = root.get<std::string>("param-location");
            if (!std::regex_match(param_location, param_location_regex)) {
                response->write(SimpleWeb::StatusCode::client_error_bad_request, bad_input_param_location);
                return;
            }
            device_name = root.get<std::string>("device-name");
            param_name = root.get<std::string>("param-name");
        } catch(...) {
            response->write(SimpleWeb::StatusCode::client_error_bad_request, bad_input_members);
            return;
        }

        try {
            boost::lock_guard<boost::mutex> lock(getObservables().lock);
            if (getObservables().get(handle).param_location != param_location) {
                getObservables().get(handle).param_location = param_location;
                getObservables().get(handle).clearValues();
            }
            getObservables().get(handle).device_name = device_name;
            getObservables().get(handle).param_name = param_name;
            boost::property_tree::ptree root; root.put("result", "success");
            send_json_response(response, root);
        } catch(...) {
            response->write(SimpleWeb::StatusCode::client_error_not_found, bad_observable_handle);
            return;
        }
    };

    server.resource["^/remove-observable/([0-9]+)$"]["POST"] = [](std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {
        boost::property_tree::ptree root; const size_t handle = std::stoi(request->path_match[1].str());
        try {
            boost::lock_guard<boost::mutex> lock(getObservables().lock);
            getObservables().remove(handle);
            root.put("result", "success");
            send_json_response(response, root);
        } catch(...) {
            response->write(SimpleWeb::StatusCode::client_error_not_found, bad_observable_handle);
            return;
        }
    };

    server.resource["^/observe-once$"]["POST"] = [](std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {
        boost::property_tree::ptree root;

        std::smatch device_location_details;
        std::string device_location;
        try { // dispatch request
            try { boost::property_tree::json_parser::read_json(request->content, root); } catch (...) {
                response->write(SimpleWeb::StatusCode::client_error_bad_request, bad_input_format);
                return;
            }
            device_location = root.get<std::string>("device-location");
            if (!std::regex_match(device_location, device_location_details, device_location_regex)) {
                response->write(SimpleWeb::StatusCode::client_error_bad_request, bad_input_device_location);
                return;
            }
        } catch(...) {
            response->write(SimpleWeb::StatusCode::client_error_bad_request, bad_input_members);
            return;
        }

        if (std::string("http").compare(device_location_details[1]) == 0) {
            send_json_response(response, http_response2json(device_location_details[2], device_location_details[3]));
        } else if (std::string("https").compare(device_location_details[1]) == 0) {
            send_json_response(response, https_response2json(device_location_details[2], device_location_details[3]));
        }
    };

    server.resource["^/get-iotdata$"]["GET"] = [](std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {
        boost::property_tree::ptree root, observables;
        do {
            boost::lock_guard<boost::mutex> lock(getObservables().lock);
            for (const auto &observable : getObservables().collect())
                observables.push_back(std::make_pair("", observable2json(observable.first, observable.second)));
        } while(0);
        root.add_child("observables", observables);
        send_json_response(response, root);
    };

    server.resource["^/get-iotdata/([0-9]+)$"]["GET"] = [](std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {
        const size_t handle = std::stoi(request->path_match[1].str());
        try {
            boost::lock_guard<boost::mutex> lock(getObservables().lock);
            send_json_response(response, observable2json(getObservables().get(handle), handle));
        } catch(...) {
            response->write(SimpleWeb::StatusCode::client_error_not_found, bad_observable_handle);
            return;
        }
    };

    // Will respond with content in the web/-directory, and its subdirectories. Default file: index.html
    // Can for instance be used to retrieve an HTML 5 client that uses REST-resources on this server
    server.default_resource["GET"] = [](std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {
        try {
            auto web_root_path = boost::filesystem::canonical("web");
            auto path = boost::filesystem::canonical(web_root_path / request->path);
            // Check if path is within web_root_path
            if(std::distance(web_root_path.begin(), web_root_path.end()) > std::distance(path.begin(), path.end()) ||
               !std::equal(web_root_path.begin(), web_root_path.end(), path.begin()))
                throw std::invalid_argument("path must be within root path");
            if(boost::filesystem::is_directory(path))
                path /= "index.html";

            SimpleWeb::CaseInsensitiveMultimap header;
            header.emplace("Cache-Control", "max-age=86400");

#ifdef HAVE_OPENSSL
    /*{
        std::ifstream ifs(path.string(), std::ifstream::in | std::ios::binary);
        if(ifs) {
            auto hash = SimpleWeb::Crypto::to_hex_string(SimpleWeb::Crypto::md5(ifs));
            header.emplace("ETag", "\"" + hash + "\"");
            auto it = request->header.find("If-None-Match");
            if(it != request->header.end()) {
                if(!it->second.empty() && it->second.compare(1, hash.size(), hash) == 0) {
                    response->write(SimpleWeb::StatusCode::redirection_not_modified, header);
                    return;
                }
            }
        }
        else throw std::invalid_argument("could not read file");
    }*/
#endif

            auto ifs = std::make_shared<std::ifstream>();
            ifs->open(path.string(), std::ifstream::in | std::ios::binary | std::ios::ate);

            if(*ifs) {
                auto length = ifs->tellg();
                ifs->seekg(0, std::ios::beg);

                header.emplace("Content-Length", std::to_string(length));
                response->write(header);

                // Trick to define a recursive function within this scope (for example purposes)
                class FileServer {
                public:
                    static void read_and_send(const std::shared_ptr<HttpServer::Response> &response, const std::shared_ptr<std::ifstream> &ifs) {
                        // Read and send 128 KB at a time
                        static std::vector<char> buffer(131072); // Safe when server is running on one thread
                        std::streamsize read_length;
                        if((read_length = ifs->read(&buffer[0], static_cast<std::streamsize>(buffer.size())).gcount()) > 0) {
                            response->write(&buffer[0], read_length);
                            if(read_length == static_cast<std::streamsize>(buffer.size())) {
                                response->send([response, ifs](const SimpleWeb::error_code &ec) {
                                    if(!ec) read_and_send(response, ifs);
                                    else std::cout << "Connection interrupted\n";
                                });
                            }
                        }
                    }
                };
                FileServer::read_and_send(response, ifs);
            }
            else
                throw std::invalid_argument("could not read file");
        }
        catch(const std::exception &e) {
            response->write(SimpleWeb::StatusCode::client_error_not_found, "Could not open path " + request->path);
        }
    };

    server.on_error = [](std::shared_ptr<HttpServer::Request> /*request*/, const SimpleWeb::error_code & /*ec*/) {
        // Handle errors here
        // Note that connection timeouts will also call this handle with ec set to SimpleWeb::errc::operation_canceled
    };

    // Start server and receive assigned port when server is listening for requests
    std::promise<unsigned short> server_port;
    std::thread server_thread([&server, &server_port]() {
        // Start server
        server.start([&server_port](unsigned short port) {
            server_port.set_value(port);
        });
    });
    std::cout << "HTTPS server is on port " << server_port.get_future().get() << '\n' << std::flush;
    ++load_progress;

    server_thread.join();
    return;
}
