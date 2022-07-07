#include "Pathfind.h"

#include <algorithm>
#include <unordered_set>
#include <utility>

#include "../Assert.h"

// BFS search path algorithm

std::vector<Coordinates> RandomPath(const std::vector<Coordinates>& start, const std::vector<Coordinates>& finish, const TilesVec& world, RNG& rng) {
    LOG_ASSERT(!start.empty() && !finish.empty());

    auto startSet = std::unordered_set<Coordinates, Coordinates::HashFunction>(start.begin(), start.end());
    auto finishSet = std::unordered_set<Coordinates, Coordinates::HashFunction>(finish.begin(), finish.end());

    auto dimensions = world.GetDimensions();
    auto visited = Vector3D<bool>(dimensions, false);
    auto prev = Vector3D<Coordinates>(dimensions, Coordinates());

    auto current = start;
    auto foundPath = false;

    while (!current.empty() && !foundPath) {
        auto next = std::vector<Coordinates>();

        for (const auto& coords : current) {
            visited.Set(coords, true);

            for (const auto& neighbour : coords.GetNeighbours(dimensions)) {
                if (visited.GetValue(neighbour)) {
                    continue;
                }
                // Can only make paths through void and fake air, or through target (finish) tiles.
                // But dungeon does not contain fake air itself, so no need to check for it.
                auto type = world.Get(neighbour).type;
                if (type != TileType::Void && !finishSet.contains(neighbour)) {
                    continue;
                }

                if (finishSet.contains(neighbour)) {
                    foundPath = true;
                }

                visited.Set(neighbour, true);
                prev.Set(neighbour, coords);
                next.push_back(neighbour);
            }
        }

        current = std::move(next);
    }

    if (!foundPath) {
        return {};
    }

    auto finishCoords = std::vector<Coordinates>();
    for (const auto& coords : current) {
        if (finishSet.contains(coords)) {
            finishCoords.push_back(coords);
        }
    }
    LOG_ASSERT(!finishCoords.empty());

    auto pathEnd = finishCoords[0];
    if (finishCoords.size() > 1) {
        pathEnd = finishCoords[rng.IntUniform(size_t(0), finishCoords.size() - 1)];
    }
    auto currentCoords = pathEnd;
    auto path = std::vector<Coordinates>({pathEnd});
    do {
        currentCoords = prev.Get(currentCoords);
        path.push_back(currentCoords);
    } while (!startSet.contains(currentCoords));

    std::reverse(path.begin(), path.end());

    return path;
}

// place path (no stairs)

void PlacePath(const std::vector<Coordinates>& path, TilesVec& world, const Tile& wall, const Tile& air) {
    for (const auto& coords : path) {
        world.Set(coords, air);
    }

    auto dimensions = world.GetDimensions();
    for (const auto& coords : path) {
        for (const auto& adjacent : coords.GetNeighbours(dimensions)) {
            // can only override Void tiles with walls
            // note: can also override FakeAir tiles, but they are not supposed to exist in the dungeon itself
            if (world.Get(adjacent).type == TileType::Void) {
                world.Set(adjacent, wall);
            }
        }
    }
}

// A* with staircases placement support

Pathfinder::Node::Node(const Coordinates& coords) : position(coords), previous(nullptr), previousSet(), cost(0.0f) {
}

size_t Pathfinder::NodePtrHashFunction::operator()(const NodePtr node) const {
    return Coordinates::HashFunction()(node->position);
}
bool Pathfinder::NodePtrEqFunction::operator()(const NodePtr lhs, const NodePtr rhs) const {
    return lhs->position == rhs->position && lhs->cost == rhs->cost && lhs->previous == rhs->previous && lhs->previousSet == rhs->previousSet;
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
                auto dx = static_cast<int>(neighbourCoords.x) - static_cast<int>(nodeCoords.x);
                auto dy = static_cast<int>(neighbourCoords.y) - static_cast<int>(nodeCoords.y);
                auto dz = static_cast<int>(neighbourCoords.z) - static_cast<int>(nodeCoords.z);

                auto xDir = glm::clamp(dx, -1, 1);
                auto zDir = glm::clamp(dz, -1, 1);
                auto verticalOffset = Coordinates{0, size_t(dy), 0};
                auto horizontalOffset = Coordinates{size_t(xDir), 0, size_t(zDir)};

                const auto& set = nodePtr->previousSet;
                if (set.contains(nodeCoords + horizontalOffset) || set.contains(nodeCoords + horizontalOffset + horizontalOffset) ||
                    set.contains(nodeCoords + verticalOffset + horizontalOffset) ||
                    set.contains(nodeCoords + verticalOffset + horizontalOffset + horizontalOffset)) {
                    continue;
                }
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
                    auto dx = static_cast<int>(neighbourCoords.x) - static_cast<int>(nodeCoords.x);
                    auto dy = static_cast<int>(neighbourCoords.y) - static_cast<int>(nodeCoords.y);
                    auto dz = static_cast<int>(neighbourCoords.z) - static_cast<int>(nodeCoords.z);
                    auto xDir = glm::clamp(dx, -1, 1);
                    auto zDir = glm::clamp(dz, -1, 1);
                    auto verticalOffset = Coordinates{0, size_t(dy), 0};
                    auto horizontalOffset = Coordinates{size_t(xDir), 0, size_t(zDir)};
                    neighbour.previousSet.insert(nodeCoords + horizontalOffset);
                    neighbour.previousSet.insert(nodeCoords + horizontalOffset + horizontalOffset);
                    neighbour.previousSet.insert(nodeCoords + verticalOffset + horizontalOffset);
                    neighbour.previousSet.insert(nodeCoords + verticalOffset + horizontalOffset + horizontalOffset);
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

    auto vecDelta = b->position.AsVec3() - a->position.AsVec3();
    auto delta = b->position - a->position;

    // no staircase needed for now
    if (delta.y == 0) {
        const auto& tile = world.Get(b->position);

        // can only pass through void tiles and finish tiles
        if (tile.type != TileType::Void && !finishSet.contains(b->position)) {
            return pathCost;
        }

        pathCost.cost = 1 + calculateHeuristic(b, target);
        pathCost.passable = true;
        return pathCost;
    }

    // staircase is necessary
    const auto& tile1 = world.Get(a->position);
    const auto& tile2 = world.Get(b->position);

    // tile a or b is not passable => can not place staircase
    if ((tile1.type != TileType::Void && !finishSet.contains(a->position)) || (tile2.type != TileType::Void && !finishSet.contains(b->position))) {
        return pathCost;
    }

    int xDir = glm::round(glm::clamp(vecDelta.x, -1.0f, 1.0f));
    int zDir = glm::round(glm::clamp(vecDelta.z, -1.0f, 1.0f));
    LOG_ASSERT(-1 <= xDir && xDir <= 1 && -1 <= zDir && zDir <= 1);

    auto verticalOffset = Coordinates{0, delta.y, 0};
    auto horizontalOffset = Coordinates{size_t(xDir), 0, size_t(zDir)};

    const auto& t1 = world.Get(a->position + horizontalOffset);
    const auto& t2 = world.Get(a->position + horizontalOffset + horizontalOffset);
    const auto& t3 = world.Get(a->position + verticalOffset + horizontalOffset);
    const auto& t4 = world.Get(a->position + verticalOffset + horizontalOffset + horizontalOffset);

    // can not place all 4 blocks of the staircase => can not place staircase
    if (t1.type != TileType::Void || t2.type != TileType::Void || t3.type != TileType::Void || t4.type != TileType::Void) {
        return pathCost;
    }

    pathCost.cost = 100 + calculateHeuristic(b, target);
    pathCost.passable = true;
    pathCost.isStairs = true;
    return pathCost;
}

void PlacePathWithStairs(const std::vector<Coordinates>& path, TilesVec& world, const Tile& wall, const Tile& air, const Tile& stairs) {
    // place air and stairs tiles

    for (size_t i = 0; i < path.size(); ++i) {
        const auto& coords = path[i];

        world.Set(coords, air);

        if (i == 0) continue;

        const auto& prev = path[i - 1];
        auto delta = coords - prev;

        if (delta.y == 0) continue;

        auto dx = static_cast<int>(coords.x) - static_cast<int>(prev.x);
        auto dy = static_cast<int>(coords.y) - static_cast<int>(prev.y);
        auto dz = static_cast<int>(coords.z) - static_cast<int>(prev.z);
        auto xDir = glm::clamp(dx, -1, 1);
        auto zDir = glm::clamp(dz, -1, 1);
        auto verticalOffset = Coordinates{0, size_t(dy), 0};
        auto horizontalOffset = Coordinates{size_t(xDir), 0, size_t(zDir)};

        world.Set(prev + horizontalOffset, stairs);
        world.Set(prev + horizontalOffset + horizontalOffset, stairs);
        world.Set(prev + verticalOffset + horizontalOffset, stairs);
        world.Set(prev + verticalOffset + horizontalOffset + horizontalOffset, stairs);
    }

    // place wall tiles

    const auto& dimensions = world.GetDimensions();
    for (const auto& coords : path) {
        for (const auto& adjacent : coords.GetNeighbours(dimensions)) {
            // can only override Void tiles with walls
            // note: can also override FakeAir tiles, but they are not supposed to exist in the dungeon itself
            if (world.Get(adjacent).type == TileType::Void) {
                world.Set(adjacent, wall);
            }
        }
    }
}
