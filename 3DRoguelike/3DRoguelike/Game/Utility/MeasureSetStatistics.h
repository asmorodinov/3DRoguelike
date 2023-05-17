#pragma once

#include "LogDuration.h"

namespace util {

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
