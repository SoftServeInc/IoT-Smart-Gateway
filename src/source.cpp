#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>
#include <boost/atomic.hpp>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <atomic>
#include <string>
#include <chrono>
#include <vector>
#include <thread>
#include <regex>

#include <stdio.h>
#include <stdlib.h>

#include "clib/observation_module.hpp"
#include "coap-server.hpp"
#include "http-server.hpp"

std::atomic_ushort load_progress;

extern boost::property_tree::ptree observable2json(const Observable&, size_t);

/*
void export_observables(const std::string & path) {
    std::ofstream ofs;
    boost::property_tree::ptree root, observables;
    ofs.open(path, std::ofstream::out | std::ios::binary | std::ios::trunc);
    do {
        boost::lock_guard<boost::mutex> lock(getObservables().lock);
        for (const auto &observable : getObservables().collect()) {
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
            observables.push_back(std::make_pair("", root));
        }
    } while(0);
    root.add_child("observables", observables);
    boost::property_tree::write_json(ofs, root);
}

void import_observables(const std::string & path) {
}
 */

void start_tcli_server() {
    while (load_progress < 2);

    //import_observables("web/backup.json");
    std::string input;
    while (true) {
        std::cout << "iotsgw$ ";
        getline(std::cin, input);
        if (input == std::string("exit")) {
            //export_observables("web/backup.json");
            abort();
        }
    }
}

int main(int argc, char ** argv) {
    load_progress = 0;
    auto concurrentProcesses = std::vector<std::thread>();
    concurrentProcesses.emplace_back(start_coap_server);
    concurrentProcesses.emplace_back(start_http_server);
    concurrentProcesses.emplace_back(start_tcli_server);

    std::for_each(concurrentProcesses.begin(), concurrentProcesses.end(),
                      [](std::thread & fileProcess) {
                          if(fileProcess.joinable())
                              fileProcess.join();
                      });
    concurrentProcesses.clear();

    return EXIT_SUCCESS;
}
