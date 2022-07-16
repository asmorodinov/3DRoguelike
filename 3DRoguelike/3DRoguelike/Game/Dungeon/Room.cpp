#include "Room.h"

#include <algorithm>
#include <array>

void IRoom::Place(TilesVec& dungeon) {
    auto dimensions = dungeon.GetDimensions();
    LOG_ASSERT(BoxFitsIntoBox(Box{offset, size}, Box{Coordinates{0, 0, 0}, dimensions}));

    for (size_t i = 0; i < size.width; ++i) {
        for (size_t j = 0; j < size.height; ++j) {
            for (size_t k = 0; k < size.length; ++k) {
                const auto& tile = tiles.Get(i, j, k);

                // do not set dungeon tiles to FakeAir
                if (tile.type != TileType::Void && tile.type != TileType::FakeAir) {
                    dungeon.Set(offset.x + i, offset.y + j, offset.z + k, tile);
                }
            }
        }
    }
}

bool BoxFitsIntoBox(const Box& box1, const Box& box2) {
    return box1.offset.x >= box2.offset.x && box1.offset.y >= box2.offset.y && box1.offset.z >= box2.offset.z &&
           box1.offset.x + box1.size.width - 1 < box2.offset.x + box2.size.width &&
           box1.offset.y + box1.size.height - 1 < box2.offset.y + box2.size.height &&
           box1.offset.z + box1.size.length - 1 < box2.offset.z + box2.size.length;
}

std::array<size_t, 6> getMinMaxHelper(const Box& box1, const Box& box2) {
    auto maxX = std::max(box1.offset.x, box2.offset.x);
    auto minX = std::min(box1.offset.x + box1.size.width - 1, box2.offset.x + box2.size.width - 1);
    auto maxY = std::max(box1.offset.y, box2.offset.y);
    auto minY = std::min(box1.offset.y + box1.size.height - 1, box2.offset.y + box2.size.height - 1);
    auto maxZ = std::max(box1.offset.z, box2.offset.z);
    auto minZ = std::min(box1.offset.z + box1.size.length - 1, box2.offset.z + box2.size.length - 1);

    return {maxX, minX, maxY, minY, maxZ, minZ};
}

bool BoxesIntersect(const Box& box1, const Box& box2) {
    auto [maxX, minX, maxY, minY, maxZ, minZ] = getMinMaxHelper(box1, box2);
    return !(maxX > minX || maxY > minY || maxZ > minZ);
}

bool RoomsIntersect(const Room& r1, const Room& r2) {
    auto [maxX, minX, maxY, minY, maxZ, minZ] = getMinMaxHelper(Box{r1->offset, r1->size}, Box{r2->offset, r2->size});
    if (maxX > minX || maxY > minY || maxZ > minZ) {
        return false;
    }

    for (size_t i = maxX; i <= minX; ++i) {
        for (size_t j = maxY; j <= minY; ++j) {
            for (size_t k = maxZ; k <= minZ; ++k) {
                // FakeAir counts as intersection between rooms
                if (r1->tiles.Get(i - r1->offset.x, j - r1->offset.y, k - r1->offset.z).type != TileType::Void ||
                    r2->tiles.Get(i - r2->offset.x, j - r2->offset.y, k - r2->offset.z).type != TileType::Void) {
                    return true;
                }
            }
        }
    }

    return false;
}

glm::vec3 RoomCenter(const Room& room) {
    return room->offset.AsVec3() + 0.5f * glm::vec3(room->size.width, room->size.height, room->size.length);
}
Coordinates RoomCenterCoords(const Room& room) {
    return room->offset + Coordinates{room->size.width / 2, room->size.height / 2, room->size.length / 2};
}

void RectRoom::Generate(RNG& rng, SeedType seed) {
    auto width = rng.IntUniform<size_t>(13, 22);
    auto height = rng.IntUniform<size_t>(9, 12);
    auto length = rng.IntUniform<size_t>(13, 22);
    size = Dimensions{width, height, length};
    tiles = TilesVec(size, Tile{TileType::FakeAir, TileDirection::None, TextureType::None, glm::vec3(1.0f)});

    auto air = Tile{TileType::Air, TileDirection::None, TextureType::None, glm::vec3(1.0f)};
    auto wall = Tile{TileType::Block, TileDirection::None, TextureType::Texture1,
                     glm::vec3(rng.RealUniform(0.3f, 1.0f), rng.RealUniform(0.3f, 1.0f), rng.RealUniform(0.3f, 1.0f))};

    auto offset = Coordinates{1, 1, 1};

    for (size_t i = offset.x + 1; i < width - 1 - offset.x; ++i) {
        for (size_t j = offset.y + 1; j < height - 1 - offset.y; ++j) {
            for (size_t k = offset.z + 1; k < length - 1 - offset.z; ++k) {
                tiles.Set(i, j, k, air);
            }
        }
    }

    for (size_t i = offset.x; i < width - offset.x; ++i)
        for (size_t k = offset.z; k < length - offset.z; ++k) {
            tiles.Set(i, offset.y, k, wall);
            tiles.Set(i, height - 1 - offset.y, k, wall);
        }
    for (size_t j = offset.y + 1; j < height - 1 - offset.y; ++j)
        for (size_t k = offset.z; k < length - offset.z; ++k) {
            tiles.Set(offset.x, j, k, wall);
            tiles.Set(width - 1 - offset.x, j, k, wall);
        }
    for (size_t i = 1 + offset.x; i < width - 1 - offset.x; ++i)
        for (size_t j = 1 + offset.y; j < height - 1 - offset.y; ++j) {
            tiles.Set(i, j, offset.z, wall);
            tiles.Set(i, j, length - 1 - offset.z, wall);
        }

    // set edge tiles
    edgeTiles.clear();
    for (size_t i = offset.x + 1; i < width - 1 - offset.x; ++i) {
        edgeTiles.push_back(Coordinates{i, offset.y + 1, offset.z});
        edgeTiles.push_back(Coordinates{i, offset.y + 1, length - 1 - offset.z});
    }
    for (size_t k = offset.z + 1; k < length - 1 - offset.z; ++k) {
        edgeTiles.push_back(Coordinates{offset.x, offset.y + 1, k});
        edgeTiles.push_back(Coordinates{width - 1 - offset.x, offset.y + 1, k});
    }
    LOG_ASSERT(!edgeTiles.empty());
}

std::vector<Coordinates> RectRoom::GetEdgeTiles() {
    return edgeTiles;
}
