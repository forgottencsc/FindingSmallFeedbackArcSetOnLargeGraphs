#ifndef UTIL_TIMER_HPP
#define UTIL_TIMER_HPP
#include <chrono>
#include <iostream>

namespace util {
    auto now() { return std::chrono::system_clock::now(); }

    struct timer {
        decltype(now()) a;
        timer() : a(now()) {}

        ~timer() { std::cerr << std::chrono::duration_cast<std::chrono::milliseconds>(now() - a).count() << "ms" << std::endl; }
    };

}

#endif
