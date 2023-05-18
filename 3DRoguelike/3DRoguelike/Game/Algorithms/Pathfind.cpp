#include "Pathfind.h"

#include <algorithm>
#include <unordered_set>
#include <utility>

#include "../Utility/LogDuration.h"
#include "../Utility/MeasureStatistics.h"

std::uint32_t id(const glm::vec3& coords, const Dimensions& dimensions) {
    auto res = static_cast<std::uint64_t>(CoordinatesToIndex(coords, dimensions));
    // res = res * 11400714819323198549ull;
    return static_cast<std::uint32_t>(res);
}

// A* with staircases placement support

Pathfinder::Node::Node(const glm::ivec3& coords) : position(coords) {
}

size_t Pathfinder::NodePtrHashFunction::operator()(const NodePtr node) const {
    return std::hash<glm::ivec3>()(node->position);
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

Pathfinder::Pathfinder(const Dimensions& dimensions) : grid(dimensions, Node()), queue() {
    for (size_t x = 0; x < dimensions.width; ++x) {
        for (size_t y = 0; y < dimensions.height; ++y) {
            for (size_t z = 0; z < dimensions.length; ++z) {
                grid.Set(x, y, z, Node(glm::ivec3{x, y, z}));
            }
        }
    }
}

std::vector<glm::ivec3> Pathfinder::FindPath(const std::vector<glm::ivec3>& start, const std::vector<glm::ivec3>& finish, const glm::ivec3& target,
                                             const TilesVec& world) {
    LOG_DURATION("Pathfinder::FindPath");
    MEASURE_STAT(findPath);

    ResetNodes();
    queue = Queue();

    const auto& dimensions = world.GetDimensions();
    auto finishSet = std::unordered_set<glm::ivec3>(finish.begin(), finish.end());

    for (const auto& coords : start) {
        auto& node = grid.Get(coords);
        node.cost = 0.0f;
        queue.insert(&node);
    }

    while (!queue.empty()) {
        auto nodePtr = *(queue.begin());
        queue.erase(nodePtr);

        nodePtr->closed = true;

        const auto& nodeCoords = nodePtr->position;

        if (finishSet.contains(nodeCoords)) {
            return reconstructPath(nodePtr);
        }

        for (const auto& neighbourCoords : GetNeighboursWithStairs(nodeCoords)) {
            if (!IsInBounds(neighbourCoords, dimensions, {0, 1, 0})) {
                continue;
            }

            auto& neighbour = grid.Get(neighbourCoords);

            if (neighbour.closed) {
                continue;
            }
            if (nodePtr->previousSet.contains(id(neighbour.position, dimensions))) {
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
                    if (nodePtr->previousSet.contains(id(tile, dimensions))) {
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
                neighbour.previousSet.insert(id(nodeCoords, dimensions));

                if (pathCost.isStairs) {
                    auto stairsInfo = GetStairsInfo(neighbourCoords, nodeCoords);
                    for (const auto& tile : stairsInfo.stairsTiles) {
                        neighbour.previousSet.insert(id(tile, dimensions));
                    }
                }
            }
        }
    }

    return {};
}

void Pathfinder::ResetNodes() {
    LOG_DURATION("Pathfinder::ResetNodes");

    const auto& dimensions = grid.GetDimensions();
    for (size_t x = 0; x < dimensions.width; ++x) {
        for (size_t y = 0; y < dimensions.height; ++y) {
            for (size_t z = 0; z < dimensions.length; ++z) {
                auto& node = grid.Get(x, y, z);
                node.previous = nullptr;
                node.cost = std::numeric_limits<float>::infinity();
                node.previousSet.clear();
                node.closed = false;
            }
        }
    }
}

std::vector<glm::ivec3> Pathfinder::reconstructPath(NodePtr node) {
    auto res = std::vector<glm::ivec3>();

    while (node != nullptr) {
        res.push_back(node->position);
        node = node->previous;
    }

    std::reverse(res.begin(), res.end());

    return res;
}

float Pathfinder::calculateHeuristic(const NodePtr b, const glm::ivec3& target) {
    return glm::distance(glm::vec3(b->position), glm::vec3(target));
}

Pathfinder::PathCost Pathfinder::costFunction(const NodePtr a, const NodePtr b, const TilesVec& world,
                                              const std::unordered_set<glm::ivec3>& finishSet, const glm::ivec3& target) {
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

    // only stairs top part can dig through stairs blocks
    const auto& topPart = world.Get(stairsTiles[0]).type;
    if (!CanPlaceStairs(topPart) && topPart != TileType::StairsBlock) {
        return pathCost;
    }
    // only stairs bottom part can dig through stairs blocks 2
    const auto& bottomPart = world.Get(stairsTiles[1]).type;
    if (!CanPlaceStairs(bottomPart) && bottomPart != TileType::StairsBlock2) {
        return pathCost;
    }

    for (size_t i = 2; i < 4; ++i) {
        if (!CanPlaceStairs(world.Get(stairsTiles[i]).type)) {
            return pathCost;
        }
    }

    for (size_t i = 4; i < 10; ++i) {
        if (!CanBeAboveOrBelowStairs(world.Get(stairsTiles[i]).type)) {
            return pathCost;
        }
    }

    pathCost.cost = calculateHeuristic(b, target);
    for (size_t i = 0; i < 4; ++i) {
        pathCost.cost += StairsCost(world.Get(stairsTiles[i]).type);
    }

    pathCost.passable = true;
    pathCost.isStairs = true;
    return pathCost;
}

void PlacePathWithStairs(const std::vector<glm::ivec3>& path, TilesVec& world, const Tile& wall, const Tile& air, const Tile& stairsAir) {
    LOG_DURATION("Pathfinder - PlacePathWithStairs");

    // place air and stairs tiles

    auto stairsVec = std::vector<glm::ivec3>();

    auto topBlock = stairsAir;
    topBlock.type = TileType::StairsTopPart;

    auto bottomBlock = stairsAir;
    bottomBlock.type = TileType::StairsBottomPart;

    for (size_t i = 0; i < path.size(); ++i) {
        const auto& coords = path[i];

        world.Set(coords, air);

        if (i == 0) continue;

        const auto& prev = path[i - 1];
        auto delta = coords - prev;

        if (delta.y == 0) continue;

        auto stairsInfo = GetStairsInfo(coords, prev);
        topBlock.orientation = stairsInfo.orientation;
        bottomBlock.orientation = stairsInfo.orientation;

        const auto& stairsTiles = stairsInfo.stairsTiles;
        for (size_t i = 0; i < 4; ++i) {
            stairsVec.push_back(stairsTiles[i]);
        }
        world.Set(stairsTiles[0], topBlock);
        world.Set(stairsTiles[1], bottomBlock);
        for (size_t i = 2; i < 4; ++i) {
            world.Set(stairsTiles[i], stairsAir);
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

    auto stairsWall = wall;
    stairsWall.type = TileType::StairsBlock;
    stairsWall.color.r = 1.0f;

    auto stairsWall2 = wall;
    stairsWall2.type = TileType::StairsBlock2;
    stairsWall2.color.g = 1.0f;

    const auto& dimensions = world.GetDimensions();
    for (const auto& coords : totalPath) {
        const auto& tile = world.Get(coords);

        for (const auto& adjacent : GetNeighbours(coords)) {
            // note: can override Void, CorridorBlock, StairsBlock and StairsBlock2 tiles and tiles that are out of bounds
            if (!IsInBounds(adjacent, dimensions) || CanBeOverridenByCorridor(world.Get(adjacent).type)) {
                if (adjacent.y == coords.y) {
                    // StairsBlock and StairsBlock2 can not replace other stairs blocks
                    if (!IsInBounds(adjacent, dimensions) || !IsStairs(world.Get(adjacent).type)) {
                        if (tile.type == TileType::StairsTopPart) {
                            world.SetInOrOutOfBounds(adjacent, stairsWall);
                        } else if (tile.type == TileType::StairsBottomPart) {
                            world.SetInOrOutOfBounds(adjacent, stairsWall2);
                        } else {
                            world.SetInOrOutOfBounds(adjacent, corridorWall);
                        }
                    } else {
                        world.SetInOrOutOfBounds(adjacent, corridorWall);
                    }
                } else {
                    // other corridors should not be able to pass through floor or ceiling of the corridor
                    world.SetInOrOutOfBounds(adjacent, wall);
                }
            }
        }
    }
}
