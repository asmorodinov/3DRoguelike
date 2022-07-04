#pragma once

#include <random>

using Seed = unsigned int;

class RNG {
 public:
    template <typename T>
    RNG(const T& seed) : rng(seed) {
    }
    template <typename T>
    void Seed(const T& seed) {
        rng.seed(seed);
    }

    template <typename T>
    T IntUniform(T a, T b) {
        return std::uniform_int_distribution<T>(a, b)(rng);
    }
    template <typename T>
    void RealUniform(T a, T b) {
        return std::uniform_real_distribution<T>(a, b)(rng);
    }

 private:
    std::mt19937 rng;
};
