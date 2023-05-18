#pragma once

#include <chrono>
#include <iostream>
#include <optional>
#include <string>
#include <sstream>

// Measures duration of a scope and prints it to console
class LogDuration {
 public:
    explicit LogDuration(const std::string& msg = "") : message(msg + ": "), start(std::chrono::steady_clock::now()) {
    }

    ~LogDuration() {
        auto finish = std::chrono::steady_clock::now();
        std::ostringstream os;
        os << message << diff(start, finish) << " us" << std::endl;
        std::cerr << os.str();
    }

    static long long diff(std::chrono::steady_clock::time_point start, std::chrono::steady_clock::time_point finish) {
        return std::chrono::duration_cast<std::chrono::microseconds>(finish - start).count();
    }

 private:
    std::string message;
    std::chrono::steady_clock::time_point start;
};

#ifndef UNIQ_ID
#define UNIQ_ID_IMPL(lineno) _a_local_var_##lineno
#define UNIQ_ID(lineno) UNIQ_ID_IMPL(lineno)
#endif

#define LOG
#ifdef LOG
#define LOG_DURATION(message) \
    LogDuration UNIQ_ID(__LINE__) { message }
#else
#define LOG_DURATION(message)
#endif
