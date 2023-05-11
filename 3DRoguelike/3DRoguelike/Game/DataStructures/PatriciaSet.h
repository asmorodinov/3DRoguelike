#pragma once

#include <array>
#include <concepts>
#include <cstdint>
#include <limits>
#include <memory>

#include "../Assert.h"

namespace PAT {

template <std::unsigned_integral Key>
Key maskbit(Key x, Key mask) {
    return x & (mask - 1u);
}

template <std::unsigned_integral Key>
bool zero(Key x, Key mask) {
    return (x & mask) == 0;
}

// the longest common prefix, return the mask
template <std::unsigned_integral Key>
Key lcp(Key& p, Key p1, Key p2) {
    Key diff = p1 ^ p2;
    Key mask = diff & (~diff + 1u);
    p = maskbit(p1, mask);
    return mask;
}

template <std::unsigned_integral Key>
class IntPatricia;
template <std::unsigned_integral Key>
using IntPatriciaPtr = std::shared_ptr<const IntPatricia<Key>>;

template <std::unsigned_integral Key, typename Alloc, typename... Args>
IntPatriciaPtr<Key> MakePatriciaPtr(Args&&... args) {
    return std::allocate_shared<const IntPatricia<Key>>(Alloc(), args...);
}

template <std::unsigned_integral Key>
class IntPatricia {
 public:
    IntPatricia(Key k = 0) : key(k), prefix(k), mask(0), left(), right() {
    }
    IntPatricia(Key k, Key p, Key m, IntPatriciaPtr<Key> l, IntPatriciaPtr<Key> r) : key(k), prefix(p), mask(m), left(l), right(r) {
    }

    bool is_leaf() const {
        return (!left) && (!right);
    }

    bool match(Key x) const {
        return (!is_leaf()) && (maskbit(x, mask) == prefix);
    }

    template <typename Alloc>
    IntPatriciaPtr<Key> replace_child(IntPatriciaPtr<Key> x, IntPatriciaPtr<Key> y) const {
        if (left.get() == x.get()) {
            return MakePatriciaPtr<Key, Alloc>(key, prefix, mask, y, right);
        } else {
            return MakePatriciaPtr<Key, Alloc>(key, prefix, mask, left, y);
        }
    }

    Key get_key() const {
        return key;
    }
    Key get_prefix() const {
        return prefix;
    }
    Key get_mask() const {
        return mask;
    }
    IntPatriciaPtr<Key> get_left() const {
        return left;
    }
    IntPatriciaPtr<Key> get_right() const {
        return right;
    }

 private:
    // A Patricia tree is an immutable structure.
    IntPatricia(const IntPatricia&) = delete;
    IntPatricia(IntPatricia&&) = delete;
    IntPatricia& operator=(const IntPatricia&) = delete;
    IntPatricia& operator=(IntPatricia&&) = delete;

 private:
    Key key;
    Key prefix;
    Key mask;
    IntPatriciaPtr<Key> left;
    IntPatriciaPtr<Key> right;
};

template <std::unsigned_integral Key, typename Alloc>
IntPatriciaPtr<Key> branch(IntPatriciaPtr<Key> t1, IntPatriciaPtr<Key> t2) {
    Key prefix;
    const auto mask = lcp(prefix, t1->get_prefix(), t2->get_prefix());
    if (zero(t1->get_prefix(), mask)) {
        return MakePatriciaPtr<Key, Alloc>(0, prefix, mask, t1, t2);
    } else {
        return MakePatriciaPtr<Key, Alloc>(0, prefix, mask, t2, t1);
    }
}

template <std::unsigned_integral Key, typename Alloc>
IntPatriciaPtr<Key> insert(IntPatriciaPtr<Key> t, Key key) {
    if (!t) {
        return MakePatriciaPtr<Key, Alloc>(key);
    }

    IntPatriciaPtr<Key> node = t;
    IntPatriciaPtr<Key> parent;

    static std::array<IntPatriciaPtr<Key>, std::numeric_limits<Key>::digits> arr;
    size_t path_length = 0;

    while (!node->is_leaf() && node->match(key)) {
        parent = node;
        if (zero(key, node->get_mask())) {
            node = node->get_left();
        } else {
            node = node->get_right();
        }
        arr[path_length++] = node;
    }

    if (node->is_leaf() && key == node->get_key()) {
        // key already present
        return t;
    }

    // key is not present, must copy path to leaf
    IntPatriciaPtr<Key> p = branch<Key, Alloc>(node, MakePatriciaPtr<Key, Alloc>(key));
    if (!parent) {
        return p;
    }

    // path copying
    auto current = p;
    for (size_t i = path_length - 1; i-- > 0;) {
        current = arr[i]->replace_child<Alloc>(arr[i + 1], current);
    }
    return t->replace_child<Alloc>(arr[0], current);
}

template <std::unsigned_integral Key>
bool lookup(IntPatriciaPtr<Key> t, Key key) {
    if (!t) {
        return false;
    }

    while ((!t->is_leaf()) && t->match(key)) {
        if (zero(key, t->get_mask())) {
            t = t->get_left();
        } else {
            t = t->get_right();
        }
    }

    if (t->is_leaf() && t->get_key() == key) {
        return true;
    } else {
        return false;
    }
}

template <std::unsigned_integral Key, typename Alloc = std::allocator<IntPatricia<Key>>>
class IntSet {
 public:
    IntSet() = default;

    bool contains(Key key) const {
        return PAT::lookup<Key>(t, key);
    }

    void insert(Key key) {
        t = PAT::insert<Key, Alloc>(t, key);
    }

    void clear() {
        t = MakePatriciaPtr<Key, Alloc>();
    }

 private:
    IntPatriciaPtr<Key> t;
};

}  // namespace PAT
