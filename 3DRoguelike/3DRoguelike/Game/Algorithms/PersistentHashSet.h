#pragma once

#ifdef PERSISTENT

#ifdef _MSC_VER
#include <intrin.h>
#define __builtin_popcount __popcnt
#endif

#include "../../ThirdPartyLibraries/hash_trie.hpp"

template <typename T>
using PersistentHashSet = hamt::hash_trie<T>;

template <typename T>
bool Contains(const PersistentHashSet<T>& set, const T& value) {
    return set.find(value).leaf();
}

#endif

#ifndef PERSISTENT

#include <unordered_set>

template <typename T>
using PersistentHashSet = std::unordered_set<T>;

template <typename T>
bool Contains(const PersistentHashSet<T>& set, const T& value) {
    return set.contains(value);
}

#endif

template <typename T>
void Insert(PersistentHashSet<T>& set, const T& value) {
    set.insert(value);
}

template <typename T>
void Clear(PersistentHashSet<T>& set) {
    PersistentHashSet<T> temp;
    set.swap(temp);
}
