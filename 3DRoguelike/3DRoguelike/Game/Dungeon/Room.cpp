#include "Room.h"

#include <algorithm>
#include <array>

void IRoom::Place(TilesVec& dungeon) {
    auto dimensions = dungeon.GetDimensions();
    LOG_ASSERT(BoxFitsIntoBox(Box{offset, size}, Box{Coordinates{0, 0, 0}, dimensions}));

    for (size_t i = 0; i < size.width; ++i) {
        for (size_t j = 0; j < size.height; ++j) {
            for (size_t k = 0; k < size.length; ++k) {
                if (tiles.Get(i, j, k).type != TileType::Void) {
                    dungeon.Set(offset.x + i, offset.y + j, offset.z + k, tiles.Get(i, j, k));
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
                if (r1->tiles.Get(i - r1->offset.x, j - r1->offset.y, k - r1->offset.z).type != TileType::Void ||
                    r2->tiles.Get(i - r2->offset.x, j - r2->offset.y, k - r2->offset.z).type != TileType::Void) {
                    return true;
                }
            }
        }
    }

    return false;
}

void RectRoom::Generate(RNG& rng, Seed seed) {
    auto width = rng.IntUniform<size_t>(11, 20);
    auto height = rng.IntUniform<size_t>(7, 10);
    auto length = rng.IntUniform<size_t>(11, 20);
    size = Dimensions{width, height, length};
    tiles = TilesVec(size, Tile{TileType::Air, TextureType::None});

    auto wall = Tile{TileType::Block, TextureType::Texture1};

    for (size_t i = 0; i < width; ++i)
        for (size_t k = 0; k < length; ++k) {
            tiles.Set(i, 0, k, wall);
            tiles.Set(i, height - 1, k, wall);
        }
    for (size_t j = 1; j < height - 1; ++j)
        for (size_t k = 0; k < length; ++k) {
            tiles.Set(0, j, k, wall);
            tiles.Set(width - 1, j, k, wall);
        }
    for (size_t i = 1; i < width - 1; ++i)
        for (size_t j = 1; j < height - 1; ++j) {
            tiles.Set(i, j, 0, wall);
            tiles.Set(i, j, length - 1, wall);
        }
}
