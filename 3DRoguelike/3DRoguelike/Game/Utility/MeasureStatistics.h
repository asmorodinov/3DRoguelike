#pragma once

#include <limits>

#include <algorithm>

#define MEASURE_STATISTICS
// #define MEASURE_SET_STATISTICS
#define MEASURE_DUNGEON_STATISTICS
#define MEASURE_PATHFIND_STATISTICS

#include "LogDuration.h"

namespace util {

using Time = long long;
using Count = int;

static constexpr Time maxTime = std::numeric_limits<Time>::max();
static constexpr Time minTime = std::numeric_limits<Time>::min();

#define REGISTER_STAT(stat)       \
    Count stat##Count = 0;        \
    Time stat##TotalTime = 0;     \
    Time stat##MinTime = maxTime; \
    Time stat##MaxTime = minTime
#define CLEAR_STAT(name)         \
    s.##name##Count = 0;         \
    s.##name##TotalTime = 0;     \
    s.##name##MinTime = maxTime; \
    s.##name##MaxTime = minTime

// Statistics struct

struct Statistics {
    // set statistics
    REGISTER_STAT(copy);
    REGISTER_STAT(contains);
    REGISTER_STAT(insert);
    REGISTER_STAT(clear);
    // dungeon generation statistics
    REGISTER_STAT(generateDungeon);
    REGISTER_STAT(generateRooms);
    REGISTER_STAT(generateCorridors);
    // pathfind statistics
    REGISTER_STAT(findPath);

    // simple set stats
    int correct = 0;
    int falsePositive = 0;
    int falseNegative = 0;
    int all = 0;
};

Statistics& GetStatistics();
void PrintReport();
void Reset();

// Measure duration

// Measures duration of a scope, and stores result in a passed variable (increments it).
// Also tracks the number of times it was created (and desctroyed).
class MeasureDuration {
 public:
    explicit MeasureDuration(Time& totalTime, Time& minTime, Time& maxTime, Count& count)
        : totalTime(totalTime), minTime(minTime), maxTime(maxTime), count(count), start(std::chrono::steady_clock::now()) {
    }

    ~MeasureDuration() {
        auto finish = std::chrono::steady_clock::now();
        auto time = LogDuration::diff(start, finish);
        totalTime += time;
        minTime = std::min(minTime, time);
        maxTime = std::max(maxTime, time);
        ++count;
    }

 private:
    Time& totalTime;
    Time& minTime;
    Time& maxTime;

    Count& count;
    std::chrono::steady_clock::time_point start;
};

#ifdef MEASURE_STATISTICS
#define MEASURE_DURATION(total, min, max, count) \
    util::MeasureDuration UNIQ_ID(__LINE__) { total, min, max, count }
#else
#define MEASURE_DURATION(total, min, max, count)
#endif

#define MEASURE_STAT(stat)            \
    auto& _s = util::GetStatistics(); \
    MEASURE_DURATION(_s.##stat##TotalTime, _s.##stat##MinTime, _s.##stat##MaxTime, _s.##stat##Count)

// Meassure Set Statistics

// helper class for printing statistics about set performance
template <typename T, typename Set>
class MeasureStatisticsSet {
 public:
    MeasureStatisticsSet() = default;

    bool contains(const T& value) const {
        MEASURE_STAT(contains);
        return s.contains(value);
    }

    void insert(const T& value) {
        MEASURE_STAT(insert);
        s.insert(value);
    }

    void clear() {
        MEASURE_STAT(clear);
        s.clear();
    }

    MeasureStatisticsSet& operator=(const MeasureStatisticsSet& other) {
        MEASURE_STAT(copy);
        s = other.s;
        return *this;
    }

 private:
    Set s;
};

}  // namespace util
