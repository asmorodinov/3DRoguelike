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

    for (size_t i = 2; i < stairsTiles.size(); ++i) {
        if (!CanPlaceStairs(world.Get(stairsTiles[i]).type)) {
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

void PlacePathWithStairs(const std::vector<Coordinates>& path, TilesVec& world, const Tile& wall, const Tile& air, const Tile& stairsAir) {
    // place air and stairs tiles

    auto stairsVec = std::vector<Coordinates>();

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
        for (const auto& tile : stairsTiles) {
            stairsVec.push_back(tile);
        }
        world.Set(stairsTiles[0], topBlock);
        world.Set(stairsTiles[1], bottomBlock);
        for (size_t i = 2; i < stairsTiles.size(); ++i) {
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

        for (const auto& intAdjacent : coords.GetAllNeighbours()) {
            auto adjacent = Coordinates{size_t(intAdjacent.x), size_t(intAdjacent.y), size_t(intAdjacent.z)};

            // note: can override Void, CorridorBlock, StairsBlock and StairsBlock2 tiles and tiles that are out of bounds
            if (!adjacent.IsInBounds(dimensions) || CanBeOverridenByCorridor(world.Get(adjacent).type)) {
                if (adjacent.y == coords.y) {
                    // it's ok for other corridors to pass through walls (side walls), so we set the tile to corridorBlock
                    // upd: if we are horizontally adjacent to stairs top (bottom) part then set tile to StairsBlock(2)
                    if (tile.type == TileType::StairsTopPart) {
                        world.SetInOrOutOfBounds(intAdjacent, stairsWall);
                    } else if (tile.type == TileType::StairsBottomPart) {
                        world.SetInOrOutOfBounds(intAdjacent, stairsWall2);
                    } else {
                        world.SetInOrOutOfBounds(intAdjacent, corridorWall);
                    }
                } else {
                    // other corridors should not be able to pass through floor or ceiling of the corridor
                    world.SetInOrOutOfBounds(intAdjacent, wall);
                }
            }
        }
    }
}
