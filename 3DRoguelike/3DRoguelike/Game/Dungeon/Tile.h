#pragma once

#include <glm/glm.hpp>

enum struct TileType {
    FakeAir,  // empty tile outside room, needed only for room intersection code, should not be part of the dungeon
    Air,      // empty tile inside of room
    Block,    // solid tile
    Void      // empty tile, initially all tiles are void
};

enum struct TextureType { None, Texture1, Texture2 };

struct Tile {
    TileType type = TileType::Void;
    TextureType texture = TextureType::None;
    glm::vec3 color = glm::vec3(1.0f);
};