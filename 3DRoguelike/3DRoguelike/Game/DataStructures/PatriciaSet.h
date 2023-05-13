#pragma once

#include <array>
#include <concepts>
#include <cstdint>
#include <limits>
#include <memory>

#include "../Assert.h"

namespace PAT {

static constexpr std::uint32_t bitmap_mask = (1u << 5) - 1u;
static constexpr std::uint32_t prefix_mask = ~bitmap_mask;

template <std::unsigned_integral Key>
Key key_prefix(Key x) {
    return x & prefix_mask;
}
template <std::unsigned_integral Key>
Key key_bitmap(Key x) {
    return 1u << (x & bitmap_mask);
}

template <std::unsigned_integral Key>
Key maskbit(Key x, Key mask) {
    return x & ((~(mask - 1u)) ^ mask);
}

template <std::unsigned_integral Key>
bool zero(Key x, Key mask) {
    return (x & mask) == 0;
}

inline std::uint32_t highestBit(std::uint32_t i) {
    i |= (i >> 1);
    i |= (i >> 2);
    i |= (i >> 4);
    i |= (i >> 8);
    i |= (i >> 16);
    return i - (i >> 1);
}

// the longest common prefix, return the mask
template <std::unsigned_integral Key>
Key lcp(Key& p, Key p1, Key p2) {
    Key mask = highestBit(p1 ^ p2);
    p = maskbit(p1, mask);
    return mask;
}

template <std::unsigned_integral Key>
bool leaf_match(Key x, Key leaf_prefix, Key leaf_bitmap) {
    static_assert(sizeof(Key) == 4);
    return (key_prefix(x) == leaf_prefix) && ((key_bitmap(x) & leaf_bitmap) != 0);
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
    IntPatricia(Key k = 0) : prefix(key_prefix(k)), mask(0), bitmap(key_bitmap(k)), left(), right() {
    }
    IntPatricia(Key p, Key m, Key b, IntPatriciaPtr<Key> l = {}, IntPatriciaPtr<Key> r = {}) : prefix(p), mask(m), bitmap(b), left(l), right(r) {
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
            return MakePatriciaPtr<Key, Alloc>(prefix, mask, bitmap, y, right);
        } else {
            return MakePatriciaPtr<Key, Alloc>(prefix, mask, bitmap, left, y);
        }
    }

    Key get_prefix() const {
        return prefix;
    }
    Key get_mask() const {
        return mask;
    }
    Key get_bitmap() const {
        return bitmap;
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
    Key prefix;
    Key mask;
    Key bitmap;
    IntPatriciaPtr<Key> left;
    IntPatriciaPtr<Key> right;
};

template <std::unsigned_integral Key, typename Alloc>
IntPatriciaPtr<Key> branch(IntPatriciaPtr<Key> t1, IntPatriciaPtr<Key> t2) {
    Key prefix;
    const auto mask = lcp(prefix, t1->get_prefix(), t2->get_prefix());
    if (zero(t1->get_prefix(), mask)) {
        return MakePatriciaPtr<Key, Alloc>(prefix, mask, 0, t1, t2);
    } else {
        return MakePatriciaPtr<Key, Alloc>(prefix, mask, 0, t2, t1);
    }
}

template <std::unsigned_integral Key, typename Alloc>
IntPatriciaPtr<Key> insert(IntPatriciaPtr<Key> t, Key key) {
    if (!t) {
        return MakePatriciaPtr<Key, Alloc>(key);
    }

    IntPatriciaPtr<Key> node = t;
    IntPatriciaPtr<Key> parent;

    static std::array<IntPatriciaPtr<Key>, std::numeric_limits<Key>::digits - 5> arr;
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

    IntPatriciaPtr<Key> current;

    if (node->is_leaf() && (key_prefix(key) == node->get_prefix())) {
        if ((key_bitmap(key) & node->get_bitmap()) != 0) {
            // key already present
            return t;
        } else {
            // add key to existing leaf's bitmap
            current = MakePatriciaPtr<Key, Alloc>(key_prefix(key), 0u, node->get_bitmap() | key_bitmap(key));
        }
    } else {
        // create new branch
        current = branch<Key, Alloc>(node, MakePatriciaPtr<Key, Alloc>(key));
    }

    if (!parent) {
        return current;
    }

    // path copying
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

    if (t->is_leaf() && leaf_match(key, t->get_prefix(), t->get_bitmap())) {
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
