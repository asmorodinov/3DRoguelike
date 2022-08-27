#include "Room.h"

#include <algorithm>
#include <array>
#include <unordered_set>
#include <utility>

#include <glm/gtx/std_based_type.hpp>

void IRoom::Place(TilesVec& dungeon) const {
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

const std::vector<glm::ivec3>& IRoom::GetEdgeTiles() const {
    return edgeTiles;
}

const std::unordered_set<glm::ivec3>& IRoom::GetIntersectionTiles() {
    if (intersectionTiles.empty()) {
        for (int i = 0; i < size.width; ++i) {
            for (int j = 0; j < size.height; ++j) {
                for (int k = 0; k < size.length; ++k) {
                    auto coords = glm::ivec3(i, j, k);
                    if (tiles.Get(coords).type == TileType::Void) {
                        continue;
                    }

                    intersectionTiles.insert(coords);
                    for (const auto& neighbour : GetNeighbours(coords)) {
                        intersectionTiles.insert(neighbour);
                    }
                }
            }
        }
    }

    return intersectionTiles;
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

bool RoomsIntersect(const Room& r1, const Room& r2) {
    auto [min, max] = getMinMaxHelper(Box{r1->offset - 1, FromIVec3(AsIVec3(r1->size) + 2)}, Box{r2->offset - 1, FromIVec3(AsIVec3(r2->size) + 2)});
    if (max.x > min.x || max.y > min.y || max.z > min.z) {
        return false;
    }

    const auto& s1 = r1->GetIntersectionTiles();
    const auto& s2 = r2->GetIntersectionTiles();

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

// room generation code

// rect room

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

// oval room

template <typename T>
using Vector2D = std::vector<std::vector<T>>;

Vector2D<TileType> generateOval(size_t width, size_t height) {
    auto insideOval = [&](size_t x, size_t y, size_t w, size_t h) {
        return std::pow(2.0f * (x + 0.5f) / w - 1.0f, 2) + std::pow(2.0f * (y + 0.5f) / h - 1.0f, 2) <= 1.0f;
    };

    auto res = Vector2D<TileType>(width, std::vector<TileType>(height, TileType::Void));

    for (size_t i = 0; i < width; ++i) {
        for (size_t j = 0; j < height; ++j) {
            if (insideOval(i, j, width, height)) {
                res[i][j] = TileType::Block;
            }
        }
    }

    auto copy = res;
    for (size_t i = 1; i < width - 1; ++i) {
        for (size_t j = 1; j < height - 1; ++j) {
            if (copy[i][j] == TileType::Block && copy[i + 1][j] == TileType::Block && copy[i - 1][j] == TileType::Block &&
                copy[i][j + 1] == TileType::Block && copy[i][j - 1] == TileType::Block) {
                res[i][j] = TileType::Air;
            }
        }
    }

    for (size_t i = 1; i < width - 1; ++i) {
        for (size_t j = 1; j < height - 1; ++j) {
            if (res[i][j] == TileType::Block && res[i + 1][j] != TileType::Air && res[i - 1][j] != TileType::Air && res[i][j + 1] != TileType::Air &&
                res[i][j - 1] != TileType::Air) {
                res[i][j] = TileType::Void;
            }
        }
    }

    return res;
}

void OvalRoom::Generate(RNG& rng, SeedType seed) {
    auto width = rng.IntUniform<size_t>(9, 18);
    auto height = rng.IntUniform<size_t>(6, 8);
    auto length = rng.IntUniform<size_t>(9, 18);
    size = Dimensions{width, height, length};
    tiles = TilesVec(size, Tile{TileType::Void, TileOrientation::None, TextureType::None, glm::vec3(1.0f)});

    auto air = Tile{TileType::Air, TileOrientation::None, TextureType::None, glm::vec3(1.0f)};
    auto wall = Tile{TileType::Block, TileOrientation::None, TextureType::Texture1,
                     glm::vec3(rng.RealUniform(0.3f, 1.0f), rng.RealUniform(0.3f, 1.0f), rng.RealUniform(0.3f, 1.0f))};

    auto oval = generateOval(width, length);

    for (size_t i = 0; i < width; ++i) {
        for (size_t k = 0; k < length; ++k) {
            if (oval[i][k] == TileType::Void) {
                continue;
            }

            tiles.Set(i, 0, k, wall);
            tiles.Set(i, height - 1, k, wall);

            if (oval[i][k] == TileType::Block) {
                edgeTiles.push_back(glm::ivec3{i, 1, k});

                for (size_t j = 1; j < height - 1; ++j) {
                    tiles.Set(i, j, k, wall);
                }
            } else if (oval[i][k] == TileType::Air) {
                for (size_t j = 1; j < height - 1; ++j) {
                    tiles.Set(i, j, k, air);
                }
            } else {
                LOG_ASSERT(false);
            }
        }
    }
}

// ellipsoid room

void EllipsoidRoom::Generate(RNG& rng, SeedType seed) {
    auto width = rng.IntUniform<size_t>(12, 18);
    auto height = rng.IntUniform<size_t>(7, 10);
    auto length = rng.IntUniform<size_t>(12, 18);
    size = Dimensions{width, height, length};

    auto voidTile = Tile{TileType::Void, TileOrientation::None, TextureType::None, glm::vec3(1.0f)};
    tiles = TilesVec(size, voidTile);

    auto air = Tile{TileType::Air, TileOrientation::None, TextureType::None, glm::vec3(1.0f)};
    auto wall = Tile{TileType::Block, TileOrientation::None, TextureType::Texture1,
                     glm::vec3(rng.RealUniform(0.3f, 1.0f), rng.RealUniform(0.3f, 1.0f), rng.RealUniform(0.3f, 1.0f))};

    auto insideEllipsoid = [&](int x, int y, int z, size_t w, size_t h, size_t l) {
        return std::pow(2.0f * (x + 0.5f) / w - 1.0f, 2) + std::pow(2.0f * (y + 0.5f) / h - 1.0f, 2) + std::pow(2.0f * (z + 0.5f) / l - 1.0f, 2) <=
               1.0f;
    };

    for (int i = 0; i < width; ++i) {
        for (int j = 0; j < height; ++j) {
            for (int k = 0; k < length; ++k) {
                if (insideEllipsoid(i, j + height / 4, k, width, height + height / 4, length)) {
                    tiles.Set(i, j, k, wall);
                }
            }
        }
    }

    auto copy = tiles;
    for (int i = 1; i < width - 1; ++i) {
        for (int j = 1; j < height - 1; ++j) {
            for (int k = 1; k < length - 1; ++k) {
                auto coords = glm::ivec3(i, j, k);

                if (copy.Get(coords).type != wall.type) {
                    continue;
                }

                auto setToAir = true;
                for (const auto& neighbour : GetNeighbours(coords)) {
                    if (copy.Get(neighbour).type != wall.type) {
                        setToAir = false;
                        break;
                    }
                }

                if (setToAir) {
                    tiles.Set(coords, air);
                }
            }
        }
    }

    for (int i = 1; i < width - 1; ++i) {
        for (int j = 1; j < height - 1; ++j) {
            for (int k = 1; k < length - 1; ++k) {
                auto coords = glm::ivec3(i, j, k);

                if (tiles.Get(coords).type != wall.type) {
                    continue;
                }

                auto setToVoid = true;
                for (const auto& neighbour : GetNeighbours(coords)) {
                    if (tiles.Get(neighbour).type == air.type) {
                        setToVoid = false;
                        break;
                    }
                }

                if (setToVoid) {
                    tiles.Set(coords, voidTile);
                }
            }
        }
    }

    for (int i = 0; i < width; ++i) {
        for (int j = 1; j < height - 1; ++j) {
            for (int k = 0; k < length; ++k) {
                auto coords = glm::ivec3(i, j, k);

                if (tiles.Get(coords).type == wall.type && tiles.Get(coords + glm::ivec3{0, -1, 0}).type == voidTile.type &&
                    tiles.Get(coords + glm::ivec3{0, 1, 0}).type == wall.type) {
                    edgeTiles.push_back(coords);
                }
            }
        }
    }
}
