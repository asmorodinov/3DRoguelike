#include "Tile.h"

#include "../Assert.h"

bool CorridorCanPass(TileType type) {
    return type == TileType::Void || type == TileType::CorridorBlock || type == TileType::CorridorAir;
}

int CorridorCost(TileType type) {
    switch (type) {
        case TileType::Void:
            return 5;
        case TileType::CorridorAir:
            return 2;
        case TileType::CorridorBlock:
            return 3;
    }

    return 200;
}

bool CanPlaceStairs(TileType type) {
    return type == TileType::Void || type == TileType::CorridorBlock;
}

// called for every stairs tile and the final cost is the sum of all costs
int StairsCost(TileType type) {
    switch (type) {
        case TileType::Void:
            return 50;
        case TileType::CorridorBlock:
            return 40;
    }

    return 1000;
}

TileType ReverseStairsDirection(TileType type) {
    switch (type) {
        case TileType::StairsEast:
            return TileType::StairsWest;
        case TileType::StairsWest:
            return TileType::StairsEast;
        case TileType::StairsNorth:
            return TileType::StairsSouth;
        case TileType::StairsSouth:
            return TileType::StairsNorth;
    }

    LOG_ASSERT(false);
    return TileType::Void;
}
