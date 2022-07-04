#pragma once

#include <vector>

struct Coordinates {
    size_t x = 0;
    size_t y = 0;
    size_t z = 0;
};

struct Dimensions {
    size_t width = 0;
    size_t height = 0;
    size_t length = 0;
};

size_t Volume(const Dimensions& dimensions);

size_t CoordinatesToIndex(const Coordinates& coordinates, const Dimensions& dimensions);

template <typename T>
class Vector3D {
 public:
    Vector3D(const Dimensions& dimensions_ = Dimensions(), const T& init = T()) : dimensions(dimensions_), data(Volume(dimensions), init) {
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

    const Dimensions& GetDimensions() const {
        return dimensions;
    }

 private:
    Dimensions dimensions;
    std::vector<T> data;
};
