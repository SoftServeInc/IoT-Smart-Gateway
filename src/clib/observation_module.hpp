//
// Created by faitc on 10/29/2020.
//

#ifndef IOT_SMART_GATEWAY_OBSERVATION_MODULE_H
#define IOT_SMART_GATEWAY_OBSERVATION_MODULE_H

#include <chrono>
#include <iostream>
#include "boost/thread.hpp"
#include "boost/circular_buffer.hpp"

#include "simple_signal.hpp"
#include "session_manager.hpp"


class Observable {
    using ValuesBuffer = boost::circular_buffer<std::pair<std::chrono::time_point<std::chrono::system_clock>,double>>;
    ValuesBuffer values;
public:
    enum Protocol : size_t { ProtocolHTTP, ProtocolHTTPS, ProtocolCOAP, ProtocolCOAPS };
    std::chrono::time_point<std::chrono::system_clock> latest_observation;
    Protocol protocol;
    std::string hostname, path;
    std::string param_location;
    std::string device_name;
    std::string param_name;
    size_t interval;
    Observable() : latest_observation(std::chrono::system_clock::now()), protocol(Protocol::ProtocolHTTPS), values(50) {};
    void clearValues() { values.clear(); };
    [[nodiscard]] const ValuesBuffer & getValues() const { return values; }
    void addValue(double value) { values.push_back(std::make_pair(std::chrono::system_clock::now(), value)); }
};

class ObservationModule;
[[nodiscard]] ObservationModule & getObservables();

class ObservationModule : public Simple::SessionManager<Observable> {
    explicit ObservationModule() : Simple::SessionManager<Observable>(10000) {};

public:
    ObservationModule(const ObservationModule&) = delete;
    ObservationModule(ObservationModule&&) = delete;
    boost::mutex lock;
    friend ObservationModule & getObservables() {
        static ObservationModule observer;
        return observer;
    } // Singleton dirty trick (testing-friendly)
};

#endif //IOT_SMART_GATEWAY_OBSERVATION_MODULE_H
