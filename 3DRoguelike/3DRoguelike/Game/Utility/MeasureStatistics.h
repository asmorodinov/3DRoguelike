#pragma once

#define MEASURE_STATISTICS
// #define MEASURE_SET_STATISTICS

#include "LogDuration.h"

namespace util {

// Statistics struct

struct Statistics {
    int copyCount;
    int containsCount;
    int insertCount;
    int clearCount;
    long long copyTime;
    long long containsTime;
    long long insertTime;
    long long clearTime;
};

Statistics& GetStatistics();
void PrintReport();
void Reset();

// Measure duration

// Measures duration of a scope, and stores result in a passed variable (increments it).
// Also tracks the number of times it was created (and desctroyed).
class MeasureDuration {
 public:
    explicit MeasureDuration(long long& time, int& count) : time(time), count(count), start(std::chrono::steady_clock::now()) {
    }

    ~MeasureDuration() {
        auto finish = std::chrono::steady_clock::now();
        time += LogDuration::diff(start, finish);
        ++count;
    }

 private:
    long long& time;
    int& count;
    std::chrono::steady_clock::time_point start;
};

#ifdef MEASURE_STATISTICS
#define MEASURE_DURATION(time, count) \
    MeasureDuration UNIQ_ID(__LINE__) { time, count }
#else
#define MEASURE_DURATION(time, count)
#endif

// Meassure Set Statistics

// helper class for printing statistics about set performance
template <typename T, typename Set>
class MeasureStatisticsSet {
 public:
    MeasureStatisticsSet() = default;

    bool contains(const T& value) const {
        MEASURE_DURATION(stats.containsTime, stats.containsCount);
        return s.contains(value);
    }

    void insert(const T& value) {
        MEASURE_DURATION(stats.insertTime, stats.insertCount);
        s.insert(value);
    }

    void clear() {
        MEASURE_DURATION(stats.clearTime, stats.clearCount);
        s.clear();
    }

    MeasureStatisticsSet& operator=(const MeasureStatisticsSet& other) {
        MEASURE_DURATION(stats.copyTime, stats.copyCount);
        s = other.s;
        return *this;
    }

 private:
    Set s;
    Statistics& stats = GetStatistics();
};

}  // namespace util
