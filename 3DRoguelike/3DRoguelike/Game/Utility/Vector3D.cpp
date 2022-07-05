#include "Vector3D.h"

bool Coordinates::operator==(const Coordinates& other) const {
    return x == other.x && y == other.y && z == other.z;
}

Coordinates Coordinates::operator+(const Coordinates& other) const {
    return Coordinates{x + other.x, y + other.y, z + other.z};
}

std::vector<Coordinates> Coordinates::GetNeighbours(const Dimensions& dimensions) const {
    auto neighbours = std::vector<Coordinates>();
    if (x > 0) neighbours.push_back(Coordinates{x - 1, y, z});
    if (y > 0) neighbours.push_back(Coordinates{x, y - 1, z});
    if (z > 0) neighbours.push_back(Coordinates{x, y, z - 1});
    if (x < dimensions.width - 1) neighbours.push_back(Coordinates{x + 1, y, z});
    if (y < dimensions.height - 1) neighbours.push_back(Coordinates{x, y + 1, z});
    if (z < dimensions.length - 1) neighbours.push_back(Coordinates{x, y, z + 1});

    return neighbours;
}

size_t Coordinates::HashFunction::operator()(const Coordinates& coords) const {
    size_t xHash = std::hash<size_t>()(coords.x);
    size_t yHash = std::hash<size_t>()(coords.y) << 1;
    size_t zHash = std::hash<size_t>()(coords.z) << 2;
    return xHash ^ yHash ^ zHash;
}

size_t Volume(const Dimensions& dimensions) {
    return dimensions.width * dimensions.height * dimensions.length;
}

size_t CoordinatesToIndex(const Coordinates& coordinates, const Dimensions& dimensions) {
    return coordinates.x * dimensions.length * dimensions.height + coordinates.y * dimensions.length + coordinates.z;
}
