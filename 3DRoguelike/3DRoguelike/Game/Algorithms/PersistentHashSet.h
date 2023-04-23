#pragma once

#include <memory>
#include <unordered_map>
#include <unordered_set>

#include <immer/set.hpp>
#include <immer/set_transient.hpp>

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
// using PersistentHashSet = ImmerHashSet<T>;
using PersistentHashSet = SimplePersistentHashSet<T>;
