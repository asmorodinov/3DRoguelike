#pragma once

#include <algorithm>
#include <set>
#include <unordered_set>
#include <vector>
#include <functional>
#include <stack>
#include <limits>

#include "../Assert.h"
#include "../Utility/Random.h"
#include "../Dungeon/WorldGrid.h"

#include "PersistentHashSet.h"

using PrevSet = PersistentHashSet<glm::ivec3>;

class Pathfinder {
 private:
    struct Node {
        glm::ivec3 position;
        Node* previous;
        PrevSet previousSet;
        float cost;

        Node(const glm::ivec3& coords = glm::ivec3());
    };

    using NodePtr = Node*;

    struct NodePtrHashFunction {
        size_t operator()(const NodePtr node) const;
    };
    struct NodePtrEqFunction {
        bool operator()(const NodePtr lhs, const NodePtr rhs) const;
    };
    struct NodePtrCmpFunction {
        bool operator()(const NodePtr lhs, const NodePtr rhs) const;
    };

    using Queue = std::set<NodePtr, NodePtrCmpFunction>;

    struct PathCost {
        bool passable;
        float cost;
        bool isStairs;
    };

 public:
    Pathfinder(const Dimensions& dimensions);

    std::vector<glm::ivec3> FindPath(const std::vector<glm::ivec3>& start, const std::vector<glm::ivec3>& finish, const glm::ivec3& target,
                                     const TilesVec& world);

 private:
    void ResetNodes();

    std::vector<glm::ivec3> reconstructPath(NodePtr node);

    float calculateHeuristic(const NodePtr b, const glm::ivec3& target);

    PathCost costFunction(const NodePtr a, const NodePtr b, const TilesVec& world, const std::unordered_set<glm::ivec3>& finishSet,
                          const glm::ivec3& target);

 private:
    Vector3D<Node> grid;
    Queue queue;
    std::unordered_set<NodePtr, NodePtrHashFunction, NodePtrEqFunction> closed;
};

void PlacePathWithStairs(const std::vector<glm::ivec3>& path, TilesVec& world, const Tile& wall, const Tile& air, const Tile& stairs);
