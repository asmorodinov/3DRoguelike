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

class Pathfinder {
 private:
    struct Node {
        Coordinates position;
        Node* previous;
        CoordinatesSet previousSet;
        float cost;

        Node(const Coordinates& coords = Coordinates());
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

    std::vector<Coordinates> FindPath(const std::vector<Coordinates>& start, const std::vector<Coordinates>& finish, const Coordinates& target,
                                      const TilesVec& world);

 private:
    void ResetNodes();

    std::vector<Coordinates> reconstructPath(NodePtr node);

    float calculateHeuristic(const NodePtr b, const Coordinates& target);

    PathCost costFunction(const NodePtr a, const NodePtr b, const TilesVec& world, const CoordinatesSet& finishSet, const Coordinates& target);

 private:
    Vector3D<Node> grid;
    Queue queue;
    std::unordered_set<NodePtr, NodePtrHashFunction, NodePtrEqFunction> closed;
};

void PlacePathWithStairs(const std::vector<Coordinates>& path, TilesVec& world, const Tile& wall, const Tile& air, const Tile& stairs);

// BFS search algorithm with no staircases support
std::vector<Coordinates> RandomPath(const std::vector<Coordinates>& start, const std::vector<Coordinates>& finish, const TilesVec& world, RNG& rng);
void PlacePath(const std::vector<Coordinates>& path, TilesVec& world, const Tile& wall, const Tile& air);
