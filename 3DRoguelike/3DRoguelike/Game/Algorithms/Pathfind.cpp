#include "Pathfind.h"

#include <algorithm>
#include <unordered_set>
#include <utility>

#include "../Assert.h"

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
                // UPD: Can also make paths through other corridors walls.
                auto type = world.Get(neighbour).type;
                if (type != TileType::Void && type != TileType::CorridorBlock && !finishSet.contains(neighbour)) {
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