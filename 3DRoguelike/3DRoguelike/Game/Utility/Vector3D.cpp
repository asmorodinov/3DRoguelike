#include "Vector3D.h"

bool Coordinates::operator<(const Coordinates& other) const {
    if (x != other.x) {
        return x < other.x;
    }
    if (y != other.y) {
        return y < other.y;
    }
    if (z != other.z) {
        return z < other.z;
    }
    return false;
}

bool Coordinates::operator==(const Coordinates& other) const {
    return x == other.x && y == other.y && z == other.z;
}

Coordinates Coordinates::operator+(const Coordinates& other) const {
    return Coordinates{x + other.x, y + other.y, z + other.z};
}
Coordinates Coordinates::operator-(const Coordinates& other) const {
    return Coordinates{x - other.x, y - other.y, z - other.z};
}

std::vector<glm::ivec3> Coordinates::GetAllNeighbours() const {
    auto ix = static_cast<int>(x);
    auto iy = static_cast<int>(y);
    auto iz = static_cast<int>(z);
    return std::vector<glm::ivec3>{glm::ivec3(ix - 1, iy, iz), glm::ivec3(ix + 1, iy, iz), glm::ivec3(ix, iy - 1, iz),
                                   glm::ivec3(ix, iy + 1, iz), glm::ivec3(ix, iy, iz - 1), glm::ivec3(ix, iy, iz + 1)};
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

std::vector<Coordinates> Coordinates::GetNeighboursWithStairs(const Dimensions& dimensions) const {
    auto neighbours = std::vector<Coordinates>();
    auto w = dimensions.width;
    auto h = dimensions.height;
    auto l = dimensions.length;

    // horizontal movement
    if (x >= 1) neighbours.push_back(Coordinates{x - 1, y, z});
    if (x < w - 1) neighbours.push_back(Coordinates{x + 1, y, z});
    if (z >= 1) neighbours.push_back(Coordinates{x, y, z - 1});
    if (z < l - 1) neighbours.push_back(Coordinates{x, y, z + 1});

    // diagonal movement
    if (y >= 1 && x >= 3) neighbours.push_back(Coordinates{x - 3, y - 1, z});
    if (y >= 1 && x < w - 3) neighbours.push_back(Coordinates{x + 3, y - 1, z});
    if (y >= 1 && z >= 3) neighbours.push_back(Coordinates{x, y - 1, z - 3});
    if (y >= 1 && z < l - 3) neighbours.push_back(Coordinates{x, y - 1, z + 3});

    if (y < h - 1 && x >= 3) neighbours.push_back(Coordinates{x - 3, y + 1, z});
    if (y < h - 1 && x < w - 3) neighbours.push_back(Coordinates{x + 3, y + 1, z});
    if (y < h - 1 && z >= 3) neighbours.push_back(Coordinates{x, y + 1, z - 3});
    if (y < h - 1 && z < l - 3) neighbours.push_back(Coordinates{x, y + 1, z + 3});

    return neighbours;
}

glm::vec3 Coordinates::AsVec3() const {
    return glm::vec3(x, y, z);
}

glm::ivec3 Coordinates::AsIVec3() const {
    return glm::ivec3(x, y, z);
}

bool Coordinates::IsInBounds(const Dimensions& dimensions) const {
    return x < dimensions.width && y < dimensions.height && z < dimensions.length;
}

size_t Coordinates::HashFunction::operator()(const Coordinates& coords) const {
    size_t xHash = std::hash<size_t>()(coords.x);
    size_t yHash = std::hash<size_t>()(coords.y) << 1;
    size_t zHash = std::hash<size_t>()(coords.z) << 2;
    return xHash ^ yHash ^ zHash;
}

Coordinates GetVerticalOffset(const Coordinates& c1, const Coordinates& c2) {
    auto dy = static_cast<int>(c1.y) - static_cast<int>(c2.y);
    return Coordinates{0, size_t(dy), 0};
}

Coordinates GetHorizontalOffset(const Coordinates& c1, const Coordinates& c2) {
    auto dx = static_cast<int>(c1.x) - static_cast<int>(c2.x);
    auto dz = static_cast<int>(c1.z) - static_cast<int>(c2.z);
    auto xDir = glm::clamp(dx, -1, 1);
    auto zDir = glm::clamp(dz, -1, 1);
    return Coordinates{size_t(xDir), 0, size_t(zDir)};
}

size_t Volume(const Dimensions& dimensions) {
    return dimensions.width * dimensions.height * dimensions.length;
}

size_t CoordinatesToIndex(const Coordinates& coordinates, const Dimensions& dimensions) {
    return coordinates.x * dimensions.length * dimensions.height + coordinates.y * dimensions.length + coordinates.z;
}
