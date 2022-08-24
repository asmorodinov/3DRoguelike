#pragma once

#include <glm/glm.hpp>

enum struct TileType {
    Air,               // empty tile inside of room
    Block,             // solid tile
    CorridorBlock,     // block that corridors are allowed to override with air
    CorridorAir,       // corridors are allowed to pass through it
    StairsAir,         // empty stairs tile
    StairsTopPart,     // stairs block top part
    StairsBottomPart,  // stairs block bottom part
    StairsBlock,       // special solid tile, needed to avoid certain unwanted configurations of adjacent staircases
    StairsBlock2,      // another special stairs solid block
    Void               // empty tile, initially all tiles are void
};

enum struct TileOrientation {
    North,  // (dz = 1, dx = 0)
    South,  // (dz = -1, dx = 0)
    West,   // (dz = 0, dx = -1)
    East,   // (dz = 0, dx = 1)
    None    // not specified orientation (e.g. tile is symmetric, orientation does not matter)
};

bool CorridorCanPass(TileType type);
int CorridorCost(TileType type);
bool CanPlaceStairs(TileType type);
int StairsCost(TileType type);

TileOrientation ReverseTileOrientation(TileOrientation orientation);
glm::ivec3 TileOrientationToIVec3(TileOrientation orientation);

bool IsSolidBlock(TileType type);

// blocks that can be replaced with corridor wall when placing a corridor
bool CanBeOverridenByCorridor(TileType type);

bool IsStairs(TileType type);

bool CanBeAboveOrBelowStairs(TileType type);

enum struct TextureType { None, Texture1, Texture2 };

struct Tile {
    TileType type = TileType::Void;
    TileOrientation orientation = TileOrientation::None;
    TextureType texture = TextureType::None;
    glm::vec3 color = glm::vec3(1.0f);
};
