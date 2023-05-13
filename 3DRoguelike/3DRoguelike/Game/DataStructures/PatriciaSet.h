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
class IntPatriciaNode;

template <std::unsigned_integral Key>
class IntPatriciaLeaf;

template <std::unsigned_integral Key>
class IntPatriciaBranch;

template <std::unsigned_integral Key>
using IntPatriciaPtr = std::shared_ptr<const IntPatriciaNode<Key>>;

template <std::unsigned_integral Key, typename Alloc, typename... Args>
IntPatriciaPtr<Key> MakePatriciaLeafPtr(Args&&... args) {
    return std::allocate_shared<const IntPatriciaLeaf<Key>>(Alloc(), args...);
}

template <std::unsigned_integral Key, typename Alloc, typename... Args>
IntPatriciaPtr<Key> MakePatriciaBranchPtr(Args&&... args) {
    return std::allocate_shared<const IntPatriciaBranch<Key>>(Alloc(), args...);
}

template <std::unsigned_integral Key>
class IntPatriciaNode {
 public:
    using Leaf = IntPatriciaLeaf<Key>;
    using Branch = IntPatriciaBranch<Key>;

    IntPatriciaNode(bool leaf) : isLeaf(leaf) {
    }
    virtual ~IntPatriciaNode() = default;

    bool IsLeaf() const {
        return isLeaf;
    }
    bool IsBranch() const {
        return !isLeaf;
    }

    const Leaf* AsLeaf() const {
        return IsLeaf() ? static_cast<const Leaf*>(this) : nullptr;
    }
    const Branch* AsBranch() const {
        return IsBranch() ? static_cast<const Branch*>(this) : nullptr;
    }

 private:
    // A Patricia tree is an immutable structure.
    IntPatriciaNode(const IntPatriciaNode&) = delete;
    IntPatriciaNode(IntPatriciaNode&&) = delete;
    IntPatriciaNode& operator=(const IntPatriciaNode&) = delete;
    IntPatriciaNode& operator=(IntPatriciaNode&&) = delete;

 private:
    bool isLeaf = false;
};

template <std::unsigned_integral Key>
class IntPatriciaLeaf : public IntPatriciaNode<Key> {
 public:
    IntPatriciaLeaf(Key key = 0, Key bitmap = 0) : IntPatriciaNode<Key>(true), prefix(key_prefix(key)), bitmap(key_bitmap(key) | bitmap) {
    }

    Key GetPrefix() const {
        return prefix;
    }
    Key GetBitmap() const {
        return bitmap;
    }

    bool Match(Key key) const {
        return (key_prefix(key) == prefix) && ((key_bitmap(key) & bitmap) != 0);
    }

 private:
    Key prefix;
    Key bitmap;
};

template <std::unsigned_integral Key>
class IntPatriciaBranch : public IntPatriciaNode<Key> {
 public:
    IntPatriciaBranch(Key p = 0, Key m = 0, IntPatriciaPtr<Key> l = {}, IntPatriciaPtr<Key> r = {})
        : IntPatriciaNode<Key>(false), prefix(p), mask(m), left(l), right(r) {
    }

    bool Match(Key key) const {
        return maskbit(key, mask) == prefix;
    }

    template <typename Alloc>
    IntPatriciaPtr<Key> ReplaceChild(IntPatriciaPtr<Key> x, IntPatriciaPtr<Key> y) const {
        if (left.get() == x.get()) {
            return MakePatriciaBranchPtr<Key, Alloc>(prefix, mask, y, right);
        } else {
            return MakePatriciaBranchPtr<Key, Alloc>(prefix, mask, left, y);
        }
    }

    Key GetPrefix() const {
        return prefix;
    }
    Key GetMask() const {
        return mask;
    }
    IntPatriciaPtr<Key> GetLeft() const {
        return left;
    }
    IntPatriciaPtr<Key> GetRight() const {
        return right;
    }

 private:
    Key prefix;
    Key mask;
    IntPatriciaPtr<Key> left;
    IntPatriciaPtr<Key> right;
};

template <std::unsigned_integral Key, typename Alloc>
IntPatriciaPtr<Key> Branch(Key p1, IntPatriciaPtr<Key> t1, Key p2, IntPatriciaPtr<Key> t2) {
    Key prefix;
    const auto mask = lcp(prefix, p1, p2);
    if (zero(p1, mask)) {
        return MakePatriciaBranchPtr<Key, Alloc>(prefix, mask, t1, t2);
    } else {
        return MakePatriciaBranchPtr<Key, Alloc>(prefix, mask, t2, t1);
    }
}

template <std::unsigned_integral Key, typename Alloc>
IntPatriciaPtr<Key> Insert(IntPatriciaPtr<Key> t, Key key) {
    if (!t) {
        return MakePatriciaLeafPtr<Key, Alloc>(key);
    }

    IntPatriciaPtr<Key> node = t;
    IntPatriciaPtr<Key> parent;

    static std::array<IntPatriciaPtr<Key>, std::numeric_limits<Key>::digits - 5> arr;
    size_t path_length = 0;

    while (node->IsBranch()) {
        const auto branch = node->AsBranch();
        if (!branch->Match(key)) {
            break;
        }

        parent = node;

        if (zero(key, branch->GetMask())) {
            node = branch->GetLeft();
        } else {
            node = branch->GetRight();
        }

        arr[path_length++] = node;
    }

    IntPatriciaPtr<Key> current;

    if (const auto leaf = node->AsLeaf(); leaf && (key_prefix(key) == leaf->GetPrefix())) {
        if ((key_bitmap(key) & leaf->GetBitmap()) != 0) {
            // key already present
            return t;
        } else {
            // add key to existing leaf's bitmap
            current = MakePatriciaLeafPtr<Key, Alloc>(key, leaf->GetBitmap());
        }
    } else {
        // create new branch
        current = Branch<Key, Alloc>(node->IsBranch() ? node->AsBranch()->GetPrefix() : node->AsLeaf()->GetPrefix(), node, key_prefix(key),
                                     MakePatriciaLeafPtr<Key, Alloc>(key));
    }

    if (!parent) {
        return current;
    }

    // path copying
    for (size_t i = path_length - 1; i-- > 0;) {
        current = arr[i]->AsBranch()->ReplaceChild<Alloc>(arr[i + 1], current);
    }
    return t->AsBranch()->ReplaceChild<Alloc>(arr[0], current);
}

template <std::unsigned_integral Key>
bool Lookup(IntPatriciaPtr<Key> t, Key key) {
    if (!t) {
        return false;
    }

    while (t->IsBranch()) {
        const auto branch = t->AsBranch();

        if (!branch->Match(key)) {
            return false;
        }

        if (zero(key, branch->GetMask())) {
            t = branch->GetLeft();
        } else {
            t = branch->GetRight();
        }
    }

    if (auto leaf = t->AsLeaf(); leaf->Match(key)) {
        return true;
    } else {
        return false;
    }
}

template <std::unsigned_integral Key, typename Alloc = std::allocator<IntPatriciaNode<Key>>>
class IntSet {
 public:
    IntSet() = default;

    bool contains(Key key) const {
        return Lookup<Key>(t, key);
    }

    void insert(Key key) {
        t = Insert<Key, Alloc>(t, key);
    }

    void clear() {
        t = {};
    }

 private:
    IntPatriciaPtr<Key> t;
};

}  // namespace PAT
