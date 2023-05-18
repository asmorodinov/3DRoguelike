#include "MeasureStatistics.h"

#include <iostream>
#include <sstream>

namespace {

using namespace util;

std::string format(Time time) {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(6) << static_cast<float>(time) / 1e6f;
    return ss.str();
}

void printCount(const std::string& name, Count count) {
    static constexpr float secondsToMicroseconds = 1e6f;
    std::cout << name << " count: " << count << "\n";
}

void printTime(const std::string& name, Time time) {
    std::cout << name << " time: " << format(time) << "\n";
}

void printTime(const std::string& name, Time total, Time min, Time max) {
    std::cout << name << " time: " << format(total) << " | " << format(min) << " | " << format(max) << "\n";
}

}  // namespace

#define PRINT_COUNT(name) printCount(#name, s.##name##Count)
#define PRINT_TIME_WITH_TYPE(name, type) printTime(std::string(#name) + " " + #type, s.##name##type##Time)
#define PRINT_TIME(name) printTime(#name, s.##name##TotalTime, s.##name##MinTime, s.##name##MaxTime)

namespace util {

Statistics& GetStatistics() {
    static Statistics s;
    return s;
}

void PrintReport() {
    const Statistics& s = GetStatistics();

#ifdef MEASURE_SET_STATISTICS
    std::cout << "----------\n";
    PRINT_COUNT(copy);
    PRINT_COUNT(insert);
    PRINT_COUNT(contains);
    PRINT_COUNT(clear);
    PRINT_TIME(copy);
    PRINT_TIME(insert);
    PRINT_TIME(contains);
    PRINT_TIME(clear);
    std::cout << "----------\n";
#endif

#ifdef MEASURE_DUNGEON_STATISTICS
    std::cout << "----------\n";
    PRINT_COUNT(generateDungeon);
    PRINT_TIME(generateDungeon);
    PRINT_TIME(generateCorridors);
    PRINT_TIME(generateRooms);
    std::cout << "----------\n";
#endif

#ifdef MEASURE_PATHFIND_STATISTICS
    std::cout << "----------\n";
    PRINT_COUNT(findPath);
    PRINT_TIME(findPath);
    std::cout << "----------\n";
#endif

    std::cout << "all: " << s.all << " correct: " << s.correct << " false-positive: " << s.falsePositive << " false-negative: " << s.falseNegative
              << "\n";
}

void Reset() {
    Statistics& s = GetStatistics();
    CLEAR_STAT(copy);
    CLEAR_STAT(insert);
    CLEAR_STAT(contains);
    CLEAR_STAT(clear);
    CLEAR_STAT(generateDungeon);
    CLEAR_STAT(generateRooms);
    CLEAR_STAT(generateCorridors);
    CLEAR_STAT(findPath);
}

}  // namespace util