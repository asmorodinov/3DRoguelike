#pragma once

#include <vector>

struct Coordinates {
    size_t x;
    size_t y;
    size_t z;
};

struct Dimensions {
    size_t width;
    size_t height;
    size_t length;
};

size_t Volume(const Dimensions& dimensions);

size_t CoordinatesToIndex(const Coordinates& coordinates, const Dimensions& dimensions);

template <typename T>
class Vector3D {
 public:
    Vector3D(const Dimensions& dimensions_, const T& init) : dimensions(dimensions_), data(Volume(dimensions), init) {
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

 private:
    Dimensions dimensions;
    std::vector<T> data;
};
