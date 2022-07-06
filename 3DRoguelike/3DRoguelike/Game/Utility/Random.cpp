#include "Random.h"

bool RNG::RandomBool(float chance) {
    return RealUniform(0.0f, 1.0f) < chance;
}
