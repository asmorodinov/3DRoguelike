#include "Vector3D.h"

size_t Volume(const Dimensions& dimensions) {
    return dimensions.width * dimensions.height * dimensions.length;
}

size_t CoordinatesToIndex(const Coordinates& coordinates, const Dimensions& dimensions) {
    return coordinates.x * dimensions.length * dimensions.height + coordinates.y * dimensions.length + coordinates.z;
}
