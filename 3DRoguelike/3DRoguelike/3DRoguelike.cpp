#include <numeric>
#include <vector>
#include <iostream>
#include <concepts>

#include <spdlog/spdlog.h>

template <typename T>
requires std::integral<T> || std::floating_point<T>
constexpr double Average(std::vector<T> const& vec) {
    const double sum = std::accumulate(vec.begin(), vec.end(), 0.0);
    return sum / vec.size();
}

int main() {
    std::vector ints{1, 2, 3, 4, 5};

    std::cout << Average(ints) << '\n';

    spdlog::info("Welcome to spdlog!");
    spdlog::error("Some error message with arg: {}", 1);
}