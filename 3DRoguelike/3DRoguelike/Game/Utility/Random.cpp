#include "Random.h"

bool RNG::RandomBool(float chance) {
    return RealUniform(0.0f, 1.0f) < chance;
}

glm::ivec3 RNG::RandomIVec3(glm::ivec3 a, glm::ivec3 b) {
    return {IntUniform(a.x, b.x), IntUniform(a.x, b.x), IntUniform(a.x, b.x)};
}

SeedType RNG::RandomSeed() {
    return SeedType(rd());
}
