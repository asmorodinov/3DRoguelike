#pragma once

#include <array>
#include <unordered_map>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtx/std_based_type.hpp>

#include "../Dungeon/Tile.h"

struct Dimensions {
    size_t width = 0;
    size_t height = 0;
    size_t length = 0;
};

glm::ivec3 AsIVec3(const Dimensions& dimensions);
Dimensions FromIVec3(const glm::ivec3& vec);

std::array<glm::ivec3, 6> GetNeighbours(const glm::ivec3& coords);
std::array<glm::ivec3, 12> GetNeighboursWithStairs(const glm::ivec3& coords);
glm::ivec3 FromVec3(const glm::vec3& vec);

bool IsInBounds(const glm::ivec3& coords, const Dimensions& dimensions, const glm::size3& offset = glm::size3());

template <typename Container>
std::vector<glm::ivec3> TilesInBounds(const Container& tiles, const Dimensions& dimensions, const glm::size3& offset = glm::size3()) {
    auto res = std::vector<glm::ivec3>();
    for (const auto& tile : tiles) {
        if (IsInBounds(tile, dimensions, offset)) {
            res.push_back(tile);
        }
    }
    return res;
}

// calculating info about staircases

glm::ivec3 GetVerticalOffset(const glm::ivec3& c1, const glm::ivec3& c2);
glm::ivec3 GetHorizontalOffset(const glm::ivec3& c1, const glm::ivec3& c2);

struct StairsInfo {
    glm::ivec3 verticalOffset;
    glm::ivec3 horizontalOffset;
    std::array<glm::ivec3, 10> stairsTiles;  // 0 - top of the stairs, 1 - bottom of the stairs, 2, 3 - empty tiles (air), 4, 5 - blocks above
                                             // stairs, 6, 7 - blocks above stairs, 8, 9 - blocks below and above exit
    TileOrientation orientation;
};

StairsInfo GetStairsInfo(const glm::ivec3& toCoords, const glm::ivec3& fromCoords);

// utility functions

size_t Volume(const Dimensions& dimensions);

size_t CoordinatesToIndex(const glm::ivec3& coordinates, const Dimensions& dimensions);

bool operator<(const glm::ivec3& a, const glm::ivec3& b);

// Vector3D class

template <typename T>
class Vector3D {
 public:
    Vector3D(const Dimensions& dimensions_ = Dimensions(), const T& init = T())
        : dimensions(dimensions_), data(Volume(dimensions), init), outOfBounds() {
    }

    void Set(const glm::ivec3& coordinates, const T& elem) {
        data[CoordinatesToIndex(coordinates, dimensions)] = elem;
    }
    void Set(size_t x, size_t y, size_t z, const T& elem) {
        Set(glm::ivec3{x, y, z}, elem);
    }

    const T& Get(const glm::ivec3& coordinates) const {
        return data[CoordinatesToIndex(coordinates, dimensions)];
    }
    const T& Get(size_t x, size_t y, size_t z) const {
        return Get(glm::ivec3{x, y, z});
    }

    T& Get(const glm::ivec3& coordinates) {
        return data[CoordinatesToIndex(coordinates, dimensions)];
    }
    T& Get(size_t x, size_t y, size_t z) {
        return Get(glm::ivec3{x, y, z});
    }

    T GetValue(const glm::ivec3& coordinates) const {
        return data[CoordinatesToIndex(coordinates, dimensions)];
    }

    const Dimensions& GetDimensions() const {
        return dimensions;
    }

    void SetInOrOutOfBounds(const glm::ivec3& coords, const T& elem) {
        if (IsInBounds(coords, dimensions)) {
            Set(coords, elem);
        } else {
            outOfBounds[coords] = elem;
        }
    }
    T GetInOrOutOfBounds(const glm::ivec3& coords) const {
        if (IsInBounds(coords, dimensions)) {
            return GetValue(coords);
        } else {
            if (outOfBounds.contains(coords)) {
                return outOfBounds.at(coords);
            } else {
                return T();
            }
        }
    }
    const std::unordered_map<glm::ivec3, T> GetOutOfBoundsMap() const {
        return outOfBounds;
    }

 private:
    Dimensions dimensions;
    std::vector<T> data;

    std::unordered_map<glm::ivec3, T> outOfBounds;
};
