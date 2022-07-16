#pragma once

#include <glm/glm.hpp>

enum struct TileType {
    FakeAir,         // empty tile outside room, needed only for room intersection code, should not be part of the dungeon
    Air,             // empty tile inside of room
    Block,           // solid tile
    CorridorBlock,   // block that corridors are allowed to override with air
    CorridorAir,     // corridors are allowed to pass through it
    StairsAir,       // empty stairs tile
    StairsTopBlock,  // stairs tile block
    Void             // empty tile, initially all tiles are void
};

enum struct TileDirection {
    North,  // (dz = 1, dx = 0)
    South,  // (dz = -1, dx = 0)
    West,   // (dz = 0, dx = -1)
    East,   // (dz = 0, dx = 1)
    None    // not specified direction (e.g. tile is symmetric, direction does not matter)
};

bool CorridorCanPass(TileType type);
int CorridorCost(TileType type);
bool CanPlaceStairs(TileType type);
int StairsCost(TileType type);

TileDirection ReverseTileDirection(TileDirection direction);

enum struct TextureType { None, Texture1, Texture2 };

struct Tile {
    TileType type = TileType::Void;
    TileDirection direction = TileDirection::None;
    TextureType texture = TextureType::None;
    glm::vec3 color = glm::vec3(1.0f);
};
