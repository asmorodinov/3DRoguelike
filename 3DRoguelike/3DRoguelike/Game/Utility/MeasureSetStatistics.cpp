#include "MeasureSetStatistics.h"

#include <iostream>

namespace util {

Statistics& GetStatistics() {
    static Statistics s;
    return s;
}

void PrintReport() {
    const Statistics& s = GetStatistics();

#ifdef MEASURE_STATISTICS
    std::cout << "Copy count: " << s.copyCount << "\n";
    std::cout << "Insert count: " << s.insertCount << "\n";
    std::cout << "Contains count: " << s.containsCount << "\n";
    std::cout << "Clear count: " << s.clearCount << "\n";
    std::cout << "Copy time: " << s.copyTime << "us \n";
    std::cout << "Insert time: " << s.insertTime << "us \n";
    std::cout << "Contains time: " << s.containsTime << "us \n";
    std::cout << "Clear time: " << s.clearTime << "us \n";
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