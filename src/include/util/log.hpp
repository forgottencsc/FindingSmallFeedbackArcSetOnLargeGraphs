#ifndef UTIL_LOG_HPP
#define UTIL_LOG_HPP
#include <unordered_map>
#include <vector>
#include <string>
#include <sstream>

namespace util {

using std::string;
using std::unordered_map;

unordered_map<string, string> log_map;

template<class T>
void log(string key, T value) {
    log_map[key] = to_string(value);
}

template<class T>
void log(string key, const std::vector<T>& value) {
    std::stringstream ss;
    for (size_t i = 0; i < value.size(); ++i) {
        ss << to_string(v);
        if (i + 1 == value.size())
            ss << ' ';
    }
    log_map[key] = ss.str();
}

}

#endif