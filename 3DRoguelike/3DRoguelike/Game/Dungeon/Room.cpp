#include "Room.h"

#include <algorithm>
#include <array>
#include <unordered_set>
#include <utility>

#include <glm/gtx/std_based_type.hpp>

void IRoom::Place(TilesVec& dungeon) {
    auto dimensions = dungeon.GetDimensions();
    LOG_ASSERT(BoxFitsIntoBox(Box{offset, size}, Box{{0, 0, 0}, dimensions}));

    for (size_t i = 0; i < size.width; ++i) {
        for (size_t j = 0; j < size.height; ++j) {
            for (size_t k = 0; k < size.length; ++k) {
                const auto& tile = tiles.Get(i, j, k);

                if (tile.type != TileType::Void) {
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

std::pair<glm::ivec3, glm::ivec3> getMinMaxHelper(const Box& box1, const Box& box2) {
    auto min = glm::min(box1.offset + AsIVec3(box1.size) - 1, box2.offset + AsIVec3(box2.size) - 1);
    auto max = glm::max(box1.offset, box2.offset);

    return {min, max};
}

bool BoxesIntersect(const Box& box1, const Box& box2) {
    auto [min, max] = getMinMaxHelper(box1, box2);
    return !(max.x > min.x || max.y > min.y || max.z > min.z);
}

bool PointInsideBox(const glm::ivec3& coords, const Box& box) {
    return coords.x >= box.offset.x && coords.x < (box.offset.x + box.size.width) && coords.y >= box.offset.y &&
           coords.y < (box.offset.y + box.size.height) && coords.z >= box.offset.z && coords.z < (box.offset.z + box.size.length);
}

std::unordered_set<glm::ivec3> getExtendedRoomTiles(const Room& r) {
    auto tiles = std::unordered_set<glm::ivec3>();
    for (int i = 0; i < r->size.width; ++i) {
        for (int j = 0; j < r->size.height; ++j) {
            for (int k = 0; k < r->size.length; ++k) {
                auto coords = glm::ivec3(i, j, k);
                if (r->tiles.Get(coords).type == TileType::Void) {
                    continue;
                }

                tiles.insert(coords);
                for (const auto& neighbour : GetNeighbours(coords)) {
                    tiles.insert(neighbour);
                }
            }
        }
    }

    return tiles;
}

bool RoomsIntersect(const Room& r1, const Room& r2) {
    auto [min, max] = getMinMaxHelper(Box{r1->offset - 1, FromIVec3(AsIVec3(r1->size) + 2)}, Box{r2->offset - 1, FromIVec3(AsIVec3(r2->size) + 2)});
    if (max.x > min.x || max.y > min.y || max.z > min.z) {
        return false;
    }

    auto s1 = getExtendedRoomTiles(r1);
    auto s2 = getExtendedRoomTiles(r2);

    // check if sets intersect
    for (const auto& tile : s1) {
        if (s2.contains(tile)) {
            return true;
        }
    }

    return false;
}

glm::vec3 RoomCenter(const Room& room) {
    return glm::vec3(room->offset) + 0.5f * glm::vec3(room->size.width, room->size.height, room->size.length);
}
glm::ivec3 RoomCenterCoords(const Room& room) {
    return room->offset + AsIVec3(room->size) / 2;
}

void RectRoom::Generate(RNG& rng, SeedType seed) {
    auto width = rng.IntUniform<size_t>(9, 18);
    auto height = rng.IntUniform<size_t>(6, 8);
    auto length = rng.IntUniform<size_t>(9, 18);
    size = Dimensions{width, height, length};
    tiles = TilesVec(size, Tile{TileType::Void, TileOrientation::None, TextureType::None, glm::vec3(1.0f)});

    auto air = Tile{TileType::Air, TileOrientation::None, TextureType::None, glm::vec3(1.0f)};
    auto wall = Tile{TileType::Block, TileOrientation::None, TextureType::Texture1,
                     glm::vec3(rng.RealUniform(0.3f, 1.0f), rng.RealUniform(0.3f, 1.0f), rng.RealUniform(0.3f, 1.0f))};

    for (size_t i = 1; i < width - 1; ++i) {
        for (size_t j = 1; j < height - 1; ++j) {
            for (size_t k = 1; k < length - 1; ++k) {
                tiles.Set(i, j, k, air);
            }
        }
    }

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

    // set edge tiles
    edgeTiles.clear();
    for (size_t i = 1; i < width - 1; ++i) {
        edgeTiles.push_back(glm::ivec3{i, 1, 0});
        edgeTiles.push_back(glm::ivec3{i, 1, length - 1});
    }
    for (size_t k = 1; k < length - 1; ++k) {
        edgeTiles.push_back(glm::ivec3{0, 1, k});
        edgeTiles.push_back(glm::ivec3{width - 1, 1, k});
    }

    LOG_ASSERT(!edgeTiles.empty());
}

std::vector<glm::ivec3> RectRoom::GetEdgeTiles() {
    return edgeTiles;
}
