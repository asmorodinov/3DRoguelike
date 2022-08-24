#include "Vector3D.h"

#include "../Assert.h"

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

    // y >= 2, y < h -2 to leave space for blocks above and below stairs tiles

    if (y >= 2 && x >= 3) neighbours.push_back(Coordinates{x - 3, y - 1, z});
    if (y >= 2 && x < w - 3) neighbours.push_back(Coordinates{x + 3, y - 1, z});
    if (y >= 2 && z >= 3) neighbours.push_back(Coordinates{x, y - 1, z - 3});
    if (y >= 2 && z < l - 3) neighbours.push_back(Coordinates{x, y - 1, z + 3});

    if (y < h - 2 && x >= 3) neighbours.push_back(Coordinates{x - 3, y + 1, z});
    if (y < h - 2 && x < w - 3) neighbours.push_back(Coordinates{x + 3, y + 1, z});
    if (y < h - 2 && z >= 3) neighbours.push_back(Coordinates{x, y + 1, z - 3});
    if (y < h - 2 && z < l - 3) neighbours.push_back(Coordinates{x, y + 1, z + 3});

    return neighbours;
}

glm::vec3 Coordinates::AsVec3() const {
    return glm::vec3(x, y, z);
}

glm::ivec3 Coordinates::AsIVec3() const {
    return glm::ivec3(x, y, z);
}

Coordinates Coordinates::FromVec3(const glm::vec3& vec) {
    auto ivec = glm::ivec3(glm::round(vec));
    return Coordinates{size_t(ivec.x), size_t(ivec.y), size_t(ivec.z)};
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

StairsInfo GetStairsInfo(const Coordinates& toCoords, const Coordinates& fromCoords) {
    auto up = Coordinates{size_t(0), size_t(1), size_t(0)};

    auto verticalOffset = GetVerticalOffset(toCoords, fromCoords);
    auto horizontalOffset = GetHorizontalOffset(toCoords, fromCoords);
    auto delta = toCoords - fromCoords;

    LOG_ASSERT(delta.y != 0);

    auto orientation = TileOrientation::None;
    if (delta.z == 3 && delta.x == 0) {
        orientation = TileOrientation::North;
    } else if (delta.x == -3 && delta.z == 0) {
        orientation = TileOrientation::West;
    } else if (delta.z == -3 && delta.x == 0) {
        orientation = TileOrientation::South;
    } else if (delta.x == 3 && delta.z == 0) {
        orientation = TileOrientation::East;
    } else {
        LOG_ASSERT(false);
    }

    auto coords = std::array<Coordinates, 10>{};

    if (delta.y == -1) {
        coords[0] = fromCoords + verticalOffset + horizontalOffset;    // stairs top
        coords[1] = toCoords - horizontalOffset;                       // stairs bottom
        coords[2] = fromCoords + horizontalOffset;                     // empty (air)
        coords[3] = fromCoords + horizontalOffset + horizontalOffset;  // empty (air)
    } else if (delta.y == 1) {
        orientation = ReverseTileOrientation(orientation);

        coords[0] = fromCoords + horizontalOffset + horizontalOffset;  // stairs top
        coords[1] = fromCoords + horizontalOffset;                     // stairs bottom
        coords[2] = toCoords - horizontalOffset;                       // empty (air)
        coords[3] = toCoords - horizontalOffset - horizontalOffset;    // empty (air)
    } else {
        LOG_ASSERT(false);
    }

    coords[4] = coords[0] - up;  // block below stairs
    coords[5] = coords[1] - up;  // block below stairs
    coords[6] = coords[0] + up;  // block above stairs
    coords[7] = coords[1] + up;  // block above stairs
    coords[8] = toCoords - up;   // block below exit
    coords[9] = toCoords + up;   // block above exit

    return {verticalOffset, horizontalOffset, coords, orientation};
}

size_t Volume(const Dimensions& dimensions) {
    return dimensions.width * dimensions.height * dimensions.length;
}

size_t CoordinatesToIndex(const Coordinates& coordinates, const Dimensions& dimensions) {
    return coordinates.x * dimensions.length * dimensions.height + coordinates.y * dimensions.length + coordinates.z;
}
