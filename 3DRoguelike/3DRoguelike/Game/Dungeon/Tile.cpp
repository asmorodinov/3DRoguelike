#include "Tile.h"

bool CorridorCanPass(TileType type) {
    return type == TileType::Void || type == TileType::CorridorBlock || type == TileType::CorridorAir;
}

int CorridorCost(TileType type) {
    if (type == TileType::Void) {
        return 5;
    } else if (type == TileType::CorridorAir) {
        // incentivize corridors intersections
        return 2;
    } else if (type == TileType::CorridorBlock) {
        // incentivize corridors intersections
        return 3;
    } else {
        return 10;
    }
}

bool CanPlaceStairs(TileType type) {
    return type == TileType::Void || type == TileType::CorridorBlock;
}

// called for every stairs tile and the final cost is the sum of all costs
int StairsCost(TileType type) {
    if (type == TileType::Void) {
        return 25;
    } else if (type == TileType::CorridorBlock) {
        // incentivize corridors intersections
        return 20;
    } else {
        return 40;
    }
}
