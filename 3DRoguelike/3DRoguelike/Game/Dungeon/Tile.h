#pragma once

#include <glm/glm.hpp>

enum struct TileType {
    FakeAir,        // empty tile outside room, needed only for room intersection code, should not be part of the dungeon
    Air,            // empty tile inside of room
    Block,          // solid tile
    CorridorBlock,  // block that corridors are allowed to override with air
    CorridorAir,    // corridors are allowed to pass through it
    Stairs,         // stairs tile
    StairsNorth,    // stairs tile that goes north (dz = 1 , dx = 0)
    StairsSouth,    // stairs tile that goes south (dz = -1, dx = 0)
    StairsWest,     // stairs tile that goes west  (dz = 0, dx = -1)
    StairsEast,     // stairs tile that goes east  (dz = 0, dx = 1)
    Void            // empty tile, initially all tiles are void
};

bool CorridorCanPass(TileType type);
int CorridorCost(TileType type);
bool CanPlaceStairs(TileType type);
int StairsCost(TileType type);

TileType ReverseStairsDirection(TileType type);

bool IsAir(TileType type);

enum struct TextureType { None, Texture1, Texture2 };

struct Tile {
    TileType type = TileType::Void;
    TextureType texture = TextureType::None;
    glm::vec3 color = glm::vec3(1.0f);
};