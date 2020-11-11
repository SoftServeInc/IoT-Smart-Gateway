//
// Created by faitc on 10/29/2020.
//

#ifndef IOT_SMART_GATEWAY_GLOBAL_TIMER_H
#define IOT_SMART_GATEWAY_GLOBAL_TIMER_H

#include <chrono>
#include <thread>
#include <iostream>
#include "simple_signal.hpp"

class GlobalTimer;
[[nodiscard]] GlobalTimer & getTimer();

class GlobalTimer : private std::thread, public Simple::Signal<void()> {
    explicit GlobalTimer() : std::thread(&GlobalTimer::execute, this) {};

    [[noreturn]] void execute() {
        using namespace std::chrono;
        auto basis = system_clock::now();
        for (;;) {
            auto secs = duration_cast<seconds>(system_clock::now()-basis).count();
            if (secs >= 1) {
                emit(); basis = system_clock::now();
            }
        }
    }

public:
    GlobalTimer(const GlobalTimer&) = delete;
    GlobalTimer(GlobalTimer&&) = delete;
    friend GlobalTimer & getTimer() {
        static GlobalTimer timer;
        return timer;
    } // Singleton dirty trick (testing-friendly)
};


#endif //IOT_SMART_GATEWAY_GLOBAL_TIMER_H
