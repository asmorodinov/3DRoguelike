#include "Vector3D.h"

#include "../Assert.h"

glm::ivec3 AsIVec3(const Dimensions& dimensions) {
    return {dimensions.width, dimensions.height, dimensions.length};
}

std::array<glm::ivec3, 6> GetNeighbours(const glm::ivec3& coords) {
    return {coords + glm::ivec3(-1, 0, 0), coords + glm::ivec3(1, 0, 0),  coords + glm::ivec3(0, -1, 0),
            coords + glm::ivec3(0, 1, 0),  coords + glm::ivec3(0, 0, -1), coords + glm::ivec3(0, 0, 1)};
}

std::array<glm::ivec3, 12> GetNeighboursWithStairs(const glm::ivec3& coords) {
    return {coords + glm::ivec3(-1, 0, 0),  coords + glm::ivec3(1, 0, 0),  coords + glm::ivec3(0, 0, -1),  coords + glm::ivec3(0, 0, 1),
            coords + glm::ivec3(-3, -1, 0), coords + glm::ivec3(3, -1, 0), coords + glm::ivec3(0, -1, -3), coords + glm::ivec3(0, -1, 3),
            coords + glm::ivec3(-3, 1, 0),  coords + glm::ivec3(3, 1, 0),  coords + glm::ivec3(0, 1, -3),  coords + glm::ivec3(0, 1, 3)};
}

glm::ivec3 FromVec3(const glm::vec3& vec) {
    return glm::ivec3(glm::round(vec));
}

bool IsInBounds(const glm::ivec3& coords, const Dimensions& dimensions, const glm::size3& offset) {
    return coords.x >= offset.x && coords.y >= offset.y && coords.z >= offset.z && coords.x < dimensions.width - offset.x &&
           coords.y < dimensions.height - offset.y && coords.z < dimensions.length - offset.z;
}

glm::ivec3 GetVerticalOffset(const glm::ivec3& c1, const glm::ivec3& c2) {
    return glm::ivec3{0, c1.y - c2.y, 0};
}

glm::ivec3 GetHorizontalOffset(const glm::ivec3& c1, const glm::ivec3& c2) {
    auto xDir = glm::clamp(c1.x - c2.x, -1, 1);
    auto zDir = glm::clamp(c1.z - c2.z, -1, 1);
    return glm::ivec3{xDir, 0, zDir};
}

StairsInfo GetStairsInfo(const glm::ivec3& toCoords, const glm::ivec3& fromCoords) {
    auto up = glm::ivec3{0, 1, 0};

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

    auto coords = std::array<glm::ivec3, 10>{};

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

bool operator<(const glm::ivec3& a, const glm::ivec3& b) {
    return std::tie(a.x, a.y, a.z) < std::tie(b.x, b.y, b.z);
}

size_t Volume(const Dimensions& dimensions) {
    return dimensions.width * dimensions.height * dimensions.length;
}

size_t CoordinatesToIndex(const glm::ivec3& coordinates, const Dimensions& dimensions) {
    return coordinates.x * dimensions.length * dimensions.height + coordinates.y * dimensions.length + coordinates.z;
}
