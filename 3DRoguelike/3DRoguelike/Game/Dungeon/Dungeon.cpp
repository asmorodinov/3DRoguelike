#include "Dungeon.h"

#include <glm/glm.hpp>

#include <vector>

Dungeon::Dungeon(const Dimensions& dimensions_, Seed seed_)
    : dimensions(dimensions_), seed(seed_), rng(seed), tiles(dimensions, Tile()), rooms(), renderer() {
}

void Dungeon::SetSeed(Seed seed_) {
    seed = seed_;
    rng.Seed(seed);
}

Seed Dungeon::GetSeed() const {
    return seed;
}

// only one room type for now
Room getRandomRoom(RNG& rng) {
    return std::make_shared<RectRoom>();
}

void Dungeon::reset() {
    rooms.clear();
    tiles = TilesVec(dimensions, Tile());
}

void Dungeon::Generate() {
    reset();

    auto tries = 1000;
    auto roomCnt = 10;

    do {
        auto newRoom = getRandomRoom(rng);
        auto roomSeed = rng.IntUniform<Seed>(Seed(0), Seed(-1));
        auto newSeed = rng.IntUniform<Seed>(Seed(0), Seed(-1));

        SetSeed(roomSeed);
        newRoom->Generate(rng, roomSeed);
        newRoom->offset = Coordinates{rng.IntUniform<size_t>(0, dimensions.width - newRoom->size.width),
                                      rng.IntUniform<size_t>(0, dimensions.height - newRoom->size.height),
                                      rng.IntUniform<size_t>(0, dimensions.length - newRoom->size.length)};
        SetSeed(newSeed);

        if (!BoxFitsIntoBox(Box{newRoom->offset, newRoom->size}, Box{Coordinates(), dimensions})) {
            continue;
        }

        auto intersect = false;
        for (const auto& room : rooms) {
            if (RoomsIntersect(room, newRoom)) {
                intersect = true;
                break;
            }
        }
        if (intersect) {
            continue;
        }

        --roomCnt;
        newRoom->Place(tiles);
        rooms.push_back(newRoom);
    } while (--tries > 0 && roomCnt != 0);

    auto tilesData = std::vector<PositionColor>();

    for (size_t x = 0; x < dimensions.width; ++x) {
        for (size_t y = 0; y < dimensions.height; ++y) {
            for (size_t z = 0; z < dimensions.width; ++z) {
                const auto& tile = tiles.Get(x, y, z);
                if (tile.type == TileType::Block && tile.texture != TextureType::None) {
                    tilesData.push_back({glm::vec3(x, y, z), tile.color});
                }
            }
        }
    }

    renderer.InitInstancedRendering(tilesData);
}

void Dungeon::Render() {
    renderer.RenderTilesInstanced();
}
