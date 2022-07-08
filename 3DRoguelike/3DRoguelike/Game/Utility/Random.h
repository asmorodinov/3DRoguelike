#pragma once

#include <random>

using SeedType = unsigned int;

class RNG {
 public:
    template <typename T>
    RNG(const T& seed) : rng(seed), rd() {
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
    T RealUniform(T a, T b) {
        return std::uniform_real_distribution<T>(a, b)(rng);
    }

    bool RandomBool(float chance = 0.5f);

 private:
    std::mt19937 rng;
    std::random_device rd;
};
