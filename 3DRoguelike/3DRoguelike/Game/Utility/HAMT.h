#pragma once

#include <array>
#include <memory>
#include <bit>
#include <cstdint>

#include "../Assert.h"

namespace HAMT {

template <typename Index>
class INode {
 public:
    using INodeT = typename INode<Index>;

 public:
    INode() = default;
    virtual ~INode() = default;

    virtual bool IsLeaf() const = 0;
    virtual std::shared_ptr<INodeT> GetChild(Index i) const = 0;
    virtual std::shared_ptr<INodeT> SetChild(Index i, const std::shared_ptr<INodeT>& node) const = 0;
    virtual std::shared_ptr<INodeT> Clone() const = 0;

    bool HasChild(Index i) const {
        return GetChildPtr(i) != nullptr;
    }

    INodeT* GetChildPtr(Index i) const {
        return GetChild(i).get();
    }
};

template <typename Index, typename Bitmask, std::uint8_t ChildrenCount>
class BranchNode : public INode<Index> {
 public:
    using INodeT = typename INode<Index>;

 public:
    BranchNode() = default;

    bool IsLeaf() const override {
        return false;
    }

    std::shared_ptr<INodeT> GetChild(Index i) const override {
        if (!BitIsSet(i)) {
            return {};
        } else {
            return children[GetIndex(i)];
        }
    }

    std::shared_ptr<INodeT> SetChild(Index i, const std::shared_ptr<INodeT>& node) const override {
        if (INode<Index>::HasChild(i)) {
            auto res = std::make_shared<BranchNode<Index, Bitmask, ChildrenCount>>(*this);
            res->children[GetIndex(i)] = node;
            return res;
        }

        auto res = std::make_shared<BranchNode<Index, Bitmask, ChildrenCount + 1>>();
        res->mask = SetBit(i);

        auto index = GetIndex(i);
        for (int j = 0; j < ChildrenCount; ++j) {
            if (j < index) {
                res->children[j] = children[j];
            } else {
                res->children[j + 1] = children[j];
            }
        }
        res->children[index] = node;

        return res;
    }

    std::shared_ptr<INodeT> Clone() const override {
        return std::make_shared<BranchNode<Index, Bitmask, ChildrenCount>>(*this);
    }

 private:
    int GetIndex(Index i) const {
        return std::popcount(((1ull << i) - 1) & mask);
    }
    bool BitIsSet(Index i) const {
        return mask & (1ull << i);
    }
    Bitmask SetBit(Index i) const {
        return mask | (1ull << i);
    }

    template <typename Index2, typename Bitmask2, std::uint8_t ChildrenCount2>
    friend class BranchNode;

 private:
    std::array<std::shared_ptr<INodeT>, ChildrenCount> children;
    Bitmask mask = 0;
};

template <typename Index>
class LeafNode : public INode<Index> {
 public:
    using INodeT = typename INode<Index>;

 public:
    LeafNode() = default;

    bool IsLeaf() const override {
        return true;
    }

    std::shared_ptr<INodeT> GetChild(Index i) const override {
        LOG_ASSERT(false);
    }

    std::shared_ptr<INodeT> SetChild(Index i, const std::shared_ptr<INodeT>& node) const override {
        LOG_ASSERT(false);
    }

    std::shared_ptr<INodeT> Clone() const override {
        return std::make_shared<LeafNode<Index>>();
    }
};

template <typename Key, typename Index, typename Bitmask, int Depth, int IndexBitsCount>
class Set {
 public:
    using INodeT = typename INode<Index>;

    template <std::uint8_t ChildrenCount>
    using BranchNodeT = BranchNode<Index, Bitmask, ChildrenCount>;

    using LeafNodeT = typename LeafNode<Index>;

 public:
    Set() = default;

    bool contains(Key key) const {
        INodeT* current = root.get();
        Index currentIndex = GetNextIndex(key);
        for (int i = 0; i < Depth; ++i) {
            if (current == nullptr) {
                return false;
            }
            current = current->GetChildPtr(currentIndex);
            currentIndex = GetNextIndex(key);
        }
        return current != nullptr && current->IsLeaf();
    }

    void insert(Key key) {
        if (contains(key)) {
            return;
        }

        root = InsertHelper(root, key, 0);
    }

    void clear() {
        root = {};
    }

 private:
    static Index GetNextIndex(Key& key) {
        Index res = key & ((1ull << IndexBitsCount) - 1);
        key >>= IndexBitsCount;
        return res;
    }

    template <typename NodeT>
    static std::shared_ptr<INodeT> Clone(INodeT* node) {
        if (!node) {
            return std::make_shared<NodeT>();
        } else {
            return node->Clone();
        }
    }

    static std::shared_ptr<INodeT> InsertHelper(const std::shared_ptr<INodeT>& root, Key key, int i) {
        if (root && root->IsLeaf()) {
            return root->Clone();
        }

        if (!root && i == Depth) {
            return std::make_shared<LeafNodeT>();
        }

        auto newRoot = Clone<BranchNodeT<0>>(root.get());
        auto currentIndex = GetNextIndex(key);
        auto child = newRoot->GetChild(currentIndex);
        auto next = InsertHelper(child, key, i + 1);

        return newRoot->SetChild(currentIndex, next);
    }

 private:
    std::shared_ptr<INodeT> root;
};

}  // namespace HAMT