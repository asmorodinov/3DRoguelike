#pragma once

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <cstdint>

#define IMMER_NO_THREAD_SAFETY 1
#include <immer/set.hpp>
#include <immer/set_transient.hpp>
#include <immer/vector.hpp>

#include "../Utility/HAMT.h"
#include "../DataStructures/PatriciaSet.h"

template <typename T>
class ImmerHashSet {
 public:
    ImmerHashSet() = default;

    bool contains(const T& value) const {
        return set.find(value) != nullptr;
    }

    void insert(const T& value) {
        set.insert(value);
    }

    void clear() {
        set = immer::set_transient<T>();
    }

 private:
    immer::set_transient<T> set;
};

template <typename T>
class ImmerPersistentHashSet {
 public:
    ImmerPersistentHashSet() = default;

    bool contains(const T& value) const {
        return set.find(value) != nullptr;
    }

    void insert(const T& value) {
        set = std::move(set).insert(value);
    }

    void clear() {
        set = Set();
    }

 private:
    using Set = immer::set<T, std::hash<T>, std::equal_to<T>, immer::default_memory_policy, 5>;
    Set set;
};

template <typename T>
class ImmerPersistentVector {
 public:
    ImmerPersistentVector() = default;

    bool contains(const T& value) const {
        return GetBit(vector.at(value / bits), value % bits);
    }

    void insert(const T& value) {
        auto val = vector.at(value / bits);
        vector = std::move(vector).set(value / bits, SetBit(val, value % bits));
    }

    void clear() {
        vector = Vector(size, 0);
    }

 private:
    static inline bool GetBit(std::uint32_t value, int bit) {
        return value & (1 << bit);
    }
    static inline std::uint32_t SetBit(std::uint32_t value, int bit) {
        return value | (1 << bit);
    }

 private:
    using Vector = immer::vector<std::uint32_t>;
    Vector vector = Vector(size, 0);

    static constexpr size_t bits = 32;
    static constexpr size_t size = (60 * 30 * 60 + bits - 1) / bits;
    static constexpr size_t elems = size * bits;
};

template <typename T>
class AlwaysEmptyHashSet {
 public:
    AlwaysEmptyHashSet() = default;

    bool contains(const T& value) const {
        return false;
    }

    void insert(const T& value) {
    }

    void clear() {
    }
};

template <typename T>
class SimplePersistentHashSet {
 public:
    SimplePersistentHashSet() = default;

    bool contains(const T& value) const {
        if (!data) {
            return false;
        }

        const auto it = data->find(value);
        if (it == data->end()) {
            return false;
        }

        return it->second < version;
    }

    void insert(const T& value) {
        if (contains(value)) {
            return;
        }

        if (!data) {
            data = std::make_shared<Map>();
        }

        data->insert({value, version++});
    }

    void clear() {
        data.reset();
        version = 0;
    }

 private:
    using Version = int;
    using Map = std::unordered_map<T, Version>;

    std::shared_ptr<Map> data;
    Version version = 0;
};

template <typename T>
// using PersistentHashSet = std::unordered_set<T>;
// using PersistentHashSet = ImmerPersistentHashSet<T>;
using PersistentHashSet = PAT::IntSet<T>;
// using PersistentHashSet = ImmerPersistentVector<T>;
// using PersistentHashSet = HAMT::Set<std::uint32_t, std::uint8_t, std::uint64_t, 5, 6>;
// using PersistentHashSet = AlwaysEmptyHashSet<T>;
// using PersistentHashSet = SimplePersistentHashSet<T>;
// using PersistentHashSet = ImmerHashSet<T>;
