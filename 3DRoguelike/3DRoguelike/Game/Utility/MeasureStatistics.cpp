#include "MeasureStatistics.h"

#include <iostream>

namespace util {

Statistics& GetStatistics() {
    static Statistics s;
    return s;
}

void PrintReport() {
    const Statistics& s = GetStatistics();

    static constexpr float secondsToMicroseconds = 1e6f;

#ifdef MEASURE_SET_STATISTICS
    std::cout << "----------\n";
    std::cout << "Copy count: " << s.copyCount << "\n";
    std::cout << "Insert count: " << s.insertCount << "\n";
    std::cout << "Contains count: " << s.containsCount << "\n";
    std::cout << "Clear count: " << s.clearCount << "\n";
    std::cout << "----------\n";
    std::cout << "Copy time: " << static_cast<float>(s.copyTime) / secondsToMicroseconds << "s \n";
    std::cout << "Insert time: " << static_cast<float>(s.insertTime) / secondsToMicroseconds << "s \n";
    std::cout << "Contains time: " << static_cast<float>(s.containsTime) / secondsToMicroseconds << "s \n";
    std::cout << "Clear time: " << static_cast<float>(s.clearTime) / secondsToMicroseconds << "s \n";
    std::cout << "----------\n";
#endif
}

void Reset() {
    Statistics& s = GetStatistics();
    s.copyCount = 0;
    s.containsCount = 0;
    s.insertCount = 0;
    s.clearCount = 0;
    s.copyTime = 0;
    s.containsTime = 0;
    s.insertTime = 0;
    s.clearTime = 0;
}

}  // namespace util