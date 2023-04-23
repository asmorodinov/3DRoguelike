#pragma once

#include <immer/set.hpp>
#include <immer/set_transient.hpp>

template <typename T>
using PersistentHashSet = immer::set_transient<T>;

template <typename T>
bool Contains(const PersistentHashSet<T>& set, const T& value) {
    return set.find(value) != nullptr;
}

template <typename T>
void Insert(PersistentHashSet<T>& set, const T& value) {
    set.insert(value);
}

template <typename T>
void Clear(PersistentHashSet<T>& set) {
    set = PersistentHashSet<T>();
}
