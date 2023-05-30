#pragma once

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <cstdint>
#include <limits>
#include <concepts>

#define IMMER_NO_THREAD_SAFETY 1
#include <immer/set.hpp>
#include <immer/set_transient.hpp>
#include <immer/vector.hpp>

#include "../../External/SPARTA/include/PatriciaTreeSet.h"
#include "../../External/ikos/core/include/ikos/core/adt/patricia_tree/set.hpp"
#include "../../External/patricia/include/sk/patricia.hxx"

#include "../Utility/HAMT.h"

#include "../../External/PersistentSet/PersistentSet/PatriciaSet.h"
#include "../../External/PersistentSet/Allocators/TwoPoolsAllocator.h"
#include "../../External/PersistentSet/Allocators/PoolAllocator.h"

#include <boost/unordered/unordered_flat_set.hpp>

#include "../Utility/MeasureStatistics.h"

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

template <std::unsigned_integral T, std::unsigned_integral Bitmap = std::uint64_t>
class ImmerPersistentVector {
 public:
    ImmerPersistentVector() : vector(EmptyVector()) {
    }

    bool contains(const T& value) const {
        return GetBit(vector.at(value / bits), value % bits);
    }

    void insert(const T& value) {
        auto val = vector.at(value / bits);
        vector = std::move(vector).set(value / bits, SetBit(val, value % bits));
    }

    void clear() {
        vector = EmptyVector();
    }

 private:
    using Vector = immer::vector<Bitmap>;

    static const Vector& EmptyVector() {
        static const Vector vec = Vector(size, 0);
        return vec;
    }

    static inline bool GetBit(Bitmap value, int bit) {
        return value & (Bitmap(1) << bit);
    }
    static inline Bitmap SetBit(Bitmap value, int bit) {
        return value | (Bitmap(1) << bit);
    }

 private:
    Vector vector;

    static constexpr size_t bits = std::numeric_limits<Bitmap>::digits;
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

        (*data)[value] = version++;
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

template <typename T, typename Alloc = std::allocator<void>>
class PersistentLinkedList {
 private:
    struct Node {
        T val;
        std::shared_ptr<const Node> next;
    };

    using NodePtr = std::shared_ptr<const Node>;

 public:
    PersistentLinkedList() = default;

    bool contains(const T& value) const {
        for (auto current = head; current; current = current->next) {
            if (current->val == value) {
                return true;
            }
        }
        return false;
    }

    void insert(const T& value) {
        head = std::allocate_shared<const Node>(Alloc(), Node{value, head});
    }

    void clear() {
        head = {};
    }

 private:
    NodePtr head;
};

template <typename T>
class SimpleChecker {
 public:
    SimpleChecker() = default;

    bool contains(const T& value) const {
        auto& s = util::GetStatistics();

        ++s.all;

        auto resGood = good.contains(value);
        auto resBad = bad.contains(value);

        if (resGood == resBad) {
            ++s.correct;
        } else if (resGood && !resBad) {
            ++s.falseNegative;
        } else {
            ++s.falsePositive;
        }

        return resGood;
    }

    void insert(const T& value) {
        good.insert(value);
        bad.insert(value);
    }

    void clear() {
        good.clear();
        bad.clear();
    }

 private:
    ImmerPersistentHashSet<T> good;
    SimplePersistentHashSet<T> bad;
};

template <typename T>
// using PersistentHashSetImpl = std::unordered_set<T>;
// using PersistentHashSetImpl = ImmerPersistentHashSet<T>;
// using PersistentHashSetImpl = SimpleChecker<T>;
//  using PersistentHashSetImpl = PersistentLinkedList<T>;
//  using PersistentHashSetImpl = PersistentLinkedList<T, StdPoolAllocator<void, 1 << 23, 40>>;
//  using PersistentHashSetImpl = boost::unordered_flat_set<T>;
//  using PersistentHashSetImpl = patricia::IntSet<T, std::uint64_t, StdTwoPoolsAllocator<void, 1 << 23, 72, 48>>;
//  using PersistentHashSetImpl = sk::patricia_set<T>;
//  using PersistentHashSetImpl = ikos::core::PatriciaTreeSet<ikos::core::Index>;
//  using PersistentHashSetImpl = sparta::PatriciaTreeSet<T>;
using PersistentHashSetImpl = ImmerPersistentVector<T>;
//  using PersistentHashSetImpl = HAMT::Set<std::uint32_t, std::uint8_t, std::uint64_t, 5, 6>;
// using PersistentHashSetImpl = AlwaysEmptyHashSet<T>;
// using PersistentHashSetImpl = SimplePersistentHashSet<T>;
// using PersistentHashSetImpl = ImmerHashSet<T>;

#ifdef MEASURE_SIMPLE_SET_CORRECTNESS_STATISTICS
template <typename T>
using PersistentHashSet = SimpleChecker<T>;
#else
#ifdef MEASURE_SET_STATISTICS
template <typename T>
using PersistentHashSet = util::MeasureStatisticsSet<T, PersistentHashSetImpl<T>>;
#else
template <typename T>
using PersistentHashSet = PersistentHashSetImpl<T>;
#endif
#endif
