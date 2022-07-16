#include "Pathfind.h"

#include <algorithm>
#include <unordered_set>
#include <utility>

#include "../Assert.h"

// A* with staircases placement support

Pathfinder::Node::Node(const Coordinates& coords) : position(coords), previous(nullptr), previousSet(), cost(0.0f) {
}

size_t Pathfinder::NodePtrHashFunction::operator()(const NodePtr node) const {
    return Coordinates::HashFunction()(node->position);
}
bool Pathfinder::NodePtrEqFunction::operator()(const NodePtr lhs, const NodePtr rhs) const {
    return lhs->position == rhs->position && lhs->cost == rhs->cost;
}
bool Pathfinder::NodePtrCmpFunction::operator()(const NodePtr lhs, const NodePtr rhs) const {
    if (lhs->cost != rhs->cost) {
        return lhs->cost < rhs->cost;
    }
    if (lhs->position != rhs->position) {
        return lhs->position < rhs->position;
    }
    // I think this should be enough info to uniquely identify nodes
    return false;
}

Pathfinder::Pathfinder(const Dimensions& dimensions) : grid(dimensions, Node()), queue(), closed() {
    for (size_t x = 0; x < dimensions.width; ++x) {
        for (size_t y = 0; y < dimensions.height; ++y) {
            for (size_t z = 0; z < dimensions.length; ++z) {
                grid.Set(x, y, z, Node(Coordinates{x, y, z}));
            }
        }
    }
}

std::vector<Coordinates> Pathfinder::FindPath(const std::vector<Coordinates>& start, const std::vector<Coordinates>& finish,
                                              const Coordinates& target, const TilesVec& world) {
    ResetNodes();
    queue = Queue();
    closed.clear();

    const auto& dimensions = world.GetDimensions();
    auto finishSet = CoordinatesSet(finish.begin(), finish.end());

    for (const auto& coords : start) {
        auto& node = grid.Get(coords);
        node.cost = 0.0f;
        queue.insert(&node);
    }

    while (!queue.empty()) {
        auto nodePtr = *(queue.begin());
        queue.erase(nodePtr);

        closed.insert(nodePtr);

        const auto& nodeCoords = nodePtr->position;

        if (finishSet.contains(nodeCoords)) {
            return reconstructPath(nodePtr);
        }

        for (const auto& neighbourCoords : nodeCoords.GetNeighboursWithStairs(dimensions)) {
            auto& neighbour = grid.Get(neighbourCoords);

            if (closed.contains(&neighbour)) {
                continue;
            }
            if (nodePtr->previousSet.contains(neighbour.position)) {
                continue;
            }

            auto pathCost = costFunction(nodePtr, &neighbour, world, finishSet, target);
            if (!pathCost.passable) {
                continue;
            }
            if (pathCost.isStairs) {
                auto stairsInfo = GetStairsInfo(neighbourCoords, nodeCoords);

                auto contains = false;
                for (const auto& tile : stairsInfo.stairsTiles) {
                    if (nodePtr->previousSet.contains(tile)) {
                        contains = true;
                        break;
                    }
                }
                if (contains) continue;
            }
            auto newCost = nodePtr->cost + pathCost.cost;

            if (newCost < neighbour.cost) {
                queue.erase(&neighbour);

                neighbour.previous = nodePtr;
                neighbour.cost = newCost;

                queue.insert(&neighbour);

                neighbour.previousSet = nodePtr->previousSet;
                neighbour.previousSet.insert(nodeCoords);

                if (pathCost.isStairs) {
                    auto stairsInfo = GetStairsInfo(neighbourCoords, nodeCoords);
                    for (const auto& tile : stairsInfo.stairsTiles) {
                        neighbour.previousSet.insert(tile);
                    }
                }
            }
        }
    }

    return {};
}

void Pathfinder::ResetNodes() {
    const auto& dimensions = grid.GetDimensions();
    for (size_t x = 0; x < dimensions.width; ++x) {
        for (size_t y = 0; y < dimensions.height; ++y) {
            for (size_t z = 0; z < dimensions.length; ++z) {
                auto& node = grid.Get(x, y, z);
                node.previous = nullptr;
                node.cost = std::numeric_limits<float>::infinity();
                node.previousSet.clear();
            }
        }
    }
}

std::vector<Coordinates> Pathfinder::reconstructPath(NodePtr node) {
    auto res = std::vector<Coordinates>();

    while (node != nullptr) {
        res.push_back(node->position);
        node = node->previous;
    }

    std::reverse(res.begin(), res.end());

    return res;
}

float Pathfinder::calculateHeuristic(const NodePtr b, const Coordinates& target) {
    return glm::distance(b->position.AsVec3(), target.AsVec3());
}

Pathfinder::PathCost Pathfinder::costFunction(const NodePtr a, const NodePtr b, const TilesVec& world, const CoordinatesSet& finishSet,
                                              const Coordinates& target) {
    auto pathCost = PathCost{false, 0.0f, false};

    auto delta = b->position - a->position;

    // no staircase needed for now
    if (delta.y == 0) {
        const auto& tile = world.Get(b->position);

        // can only pass through Void, CorridorBlock, CorridorAir, and finish tiles
        if (!CorridorCanPass(tile.type) && !finishSet.contains(b->position)) {
            return pathCost;
        }

        pathCost.cost = CorridorCost(tile.type) + calculateHeuristic(b, target);
        pathCost.passable = true;
        return pathCost;
    }

    // staircase is necessary
    const auto& tile1 = world.Get(a->position);
    const auto& tile2 = world.Get(b->position);

    // tile a or b is not passable => can not place staircase
    if ((!CorridorCanPass(tile1.type) && !finishSet.contains(a->position)) || (!CorridorCanPass(tile2.type) && !finishSet.contains(b->position))) {
        return pathCost;
    }

    auto stairsInfo = GetStairsInfo(b->position, a->position);
    const auto& stairsTiles = stairsInfo.stairsTiles;
    for (const auto& tile : stairsTiles) {
        if (!CanPlaceStairs(world.Get(tile).type)) {
            return pathCost;
        }
    }

    pathCost.cost = calculateHeuristic(b, target);
    for (const auto& tile : stairsTiles) {
        pathCost.cost += StairsCost(world.Get(tile).type);
    }

    pathCost.passable = true;
    pathCost.isStairs = true;
    return pathCost;
}

void PlacePathWithStairs(const std::vector<Coordinates>& path, TilesVec& world, const Tile& wall, const Tile& air, const Tile& stairs) {
    // place air and stairs tiles

    auto stairsVec = std::vector<Coordinates>();

    auto stairs2 = stairs;
    stairs2.type = TileType::StairsTopBlock;

    for (size_t i = 0; i < path.size(); ++i) {
        const auto& coords = path[i];

        world.Set(coords, air);

        if (i == 0) continue;

        const auto& prev = path[i - 1];
        auto delta = coords - prev;

        if (delta.y == 0) continue;

        auto stairsInfo = GetStairsInfo(coords, prev);
        stairs2.direction = stairsInfo.direction;

        const auto& stairsTiles = stairsInfo.stairsTiles;
        for (const auto& tile : stairsTiles) {
            stairsVec.push_back(tile);
        }
        world.Set(stairsTiles[0], stairs2);
        for (size_t i = 1; i < stairsTiles.size(); ++i) {
            world.Set(stairsTiles[i], stairs);
        }
    }

    // add stairs tiles to path as well
    auto totalPath = path;
    totalPath.insert(totalPath.end(), stairsVec.begin(), stairsVec.end());

    // place wall tiles

    auto corridorWall = wall;
    corridorWall.type = TileType::CorridorBlock;
    std::swap(corridorWall.color.r, corridorWall.color.g);
    corridorWall.color.b = 1.0f;

    const auto& dimensions = world.GetDimensions();
    for (const auto& coords : totalPath) {
        for (const auto& intAdjacent : coords.GetAllNeighbours()) {
            auto adjacent = Coordinates{size_t(intAdjacent.x), size_t(intAdjacent.y), size_t(intAdjacent.z)};

            // can only override Void tiles with walls
            // note: can also override FakeAir tiles, but they are not supposed to exist in the dungeon itself
            // note: some blocks may end up out of world bounds, but that's ok
            if (!adjacent.IsInBounds(dimensions) || world.Get(adjacent).type == TileType::Void) {
                if (adjacent.y == coords.y) {
                    // it's ok for other corridors to pass through walls (side walls), so we set the tile to corridorBlock
                    world.SetInOrOutOfBounds(intAdjacent, corridorWall);
                } else {
                    // other corridors should not be able to pass through floor or ceiling of the corridor
                    world.SetInOrOutOfBounds(intAdjacent, wall);
                }
            }
        }
    }
}
