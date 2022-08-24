#pragma once

#include <array>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include "../Dungeon/Tile.h"

// Dimensions and Coordinates structs

struct Dimensions {
    size_t width = 0;
    size_t height = 0;
    size_t length = 0;
};

struct Coordinates {
    size_t x = 0;
    size_t y = 0;
    size_t z = 0;

    bool operator<(const Coordinates& other) const;
    bool operator==(const Coordinates& other) const;
    Coordinates operator+(const Coordinates& other) const;
    Coordinates operator-(const Coordinates& other) const;

    struct HashFunction {
        size_t operator()(const Coordinates& coords) const;
    };

    std::vector<glm::ivec3> GetAllNeighbours() const;
    std::vector<Coordinates> GetNeighbours(const Dimensions& dimensions) const;
    std::vector<Coordinates> GetNeighboursWithStairs(const Dimensions& dimensions) const;

    glm::vec3 AsVec3() const;
    glm::ivec3 AsIVec3() const;
    static Coordinates FromVec3(const glm::vec3& vec);

    bool IsInBounds(const Dimensions& dimensions) const;
};

using CoordinatesSet = std::unordered_set<Coordinates, Coordinates::HashFunction>;

// calculating info about staircases

Coordinates GetVerticalOffset(const Coordinates& c1, const Coordinates& c2);
Coordinates GetHorizontalOffset(const Coordinates& c1, const Coordinates& c2);

struct StairsInfo {
    Coordinates verticalOffset;
    Coordinates horizontalOffset;
    std::array<Coordinates, 10> stairsTiles;  // 0 - top of the stairs, 1 - bottom of the stairs, 2, 3 - empty tiles (air), 4, 5 - blocks above
                                              // stairs, 6, 7 - blocks above stairs, 8, 9 - blocks below and above exit
    TileOrientation orientation;
};

StairsInfo GetStairsInfo(const Coordinates& toCoords, const Coordinates& fromCoords);

// utility functions

size_t Volume(const Dimensions& dimensions);

size_t CoordinatesToIndex(const Coordinates& coordinates, const Dimensions& dimensions);

// Vector3D class

template <typename T>
class Vector3D {
 public:
    Vector3D(const Dimensions& dimensions_ = Dimensions(), const T& init = T())
        : dimensions(dimensions_), data(Volume(dimensions), init), outOfBounds() {
    }

    void Set(const Coordinates& coordinates, const T& elem) {
        data[CoordinatesToIndex(coordinates, dimensions)] = elem;
    }
    void Set(size_t x, size_t y, size_t z, const T& elem) {
        Set(Coordinates{x, y, z}, elem);
    }

    const T& Get(const Coordinates& coordinates) const {
        return data[CoordinatesToIndex(coordinates, dimensions)];
    }
    const T& Get(size_t x, size_t y, size_t z) const {
        return Get(Coordinates{x, y, z});
    }

    T& Get(const Coordinates& coordinates) {
        return data[CoordinatesToIndex(coordinates, dimensions)];
    }
    T& Get(size_t x, size_t y, size_t z) {
        return Get(Coordinates{x, y, z});
    }

    T GetValue(const Coordinates& coordinates) const {
        return data[CoordinatesToIndex(coordinates, dimensions)];
    }

    const Dimensions& GetDimensions() const {
        return dimensions;
    }

    void SetInOrOutOfBounds(const glm::ivec3& intcoords, const T& elem) {
        auto coords = Coordinates{size_t(intcoords.x), size_t(intcoords.y), size_t(intcoords.z)};

        if (coords.IsInBounds(dimensions)) {
            Set(coords, elem);
        } else {
            outOfBounds[intcoords] = elem;
        }
    }
    T GetInOrOutOfBounds(const glm::ivec3& intcoords) const {
        auto coords = Coordinates{size_t(intcoords.x), size_t(intcoords.y), size_t(intcoords.z)};

        if (coords.IsInBounds(dimensions)) {
            return GetValue(coords);
        } else {
            if (outOfBounds.contains(intcoords)) {
                return outOfBounds.at(intcoords);
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
