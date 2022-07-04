#include "Dungeon.h"

#include <glm/glm.hpp>

#include <vector>

Dungeon::Dungeon(const Dimensions& dimensions_, Seed seed_) : dimensions(dimensions_), seed(seed_), world(dimensions, Tile()), renderer() {
}

void Dungeon::SetSeed(Seed seed_) {
    seed = seed_;
}

Seed Dungeon::GetSeed() const {
    return seed;
}

void Dungeon::Generate() {
    auto tiles = std::vector<glm::vec3>();

    for (size_t x = 0; x < dimensions.width; ++x) {
        for (size_t y = 0; y < dimensions.height; ++y) {
            for (size_t z = 0; z < dimensions.width; ++z) {
                if (x == 0 || y == 0 || z == 0 || x == (dimensions.width - 1) || y == (dimensions.height - 1) || z == (dimensions.length - 1)) {
                    world.Set(x, y, z, Tile{TileType::Block, TextureType::Texture1});

                    tiles.push_back(glm::vec3(x, y, z));
                } else {
                    world.Set(x, y, z, Tile{TileType::Air, TextureType::None});
                }
            }
        }
    }

    renderer.InitInstancedRendering(tiles);
}

void Dungeon::Render() {
    renderer.RenderTilesInstanced();
}
