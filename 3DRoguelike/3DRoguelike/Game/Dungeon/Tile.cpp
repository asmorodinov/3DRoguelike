#include "Tile.h"

#include "../Assert.h"

bool CorridorCanPass(TileType type) {
    return type == TileType::Void || type == TileType::CorridorBlock || type == TileType::CorridorAir;
}

int CorridorCost(TileType type) {
    switch (type) {
        case TileType::Void:
            return 10;
        case TileType::CorridorAir:
            return 6;
        case TileType::CorridorBlock:
            return 8;
    }

    return 500;
}

bool CanPlaceStairs(TileType type) {
    return type == TileType::Void || type == TileType::CorridorBlock;
}

// called for every stairs tile and the final cost is the sum of all costs
int StairsCost(TileType type) {
    switch (type) {
        case TileType::Void:
            return 100;
        case TileType::CorridorBlock:
            return 90;
        case TileType::StairsBlock:
            return 80;
        case TileType::StairsBlock2:
            return 80;
    }

    return 10000;
}

TileOrientation ReverseTileOrientation(TileOrientation orientation) {
    switch (orientation) {
        case TileOrientation::East:
            return TileOrientation::West;
        case TileOrientation::West:
            return TileOrientation::East;
        case TileOrientation::North:
            return TileOrientation::South;
        case TileOrientation::South:
            return TileOrientation::North;
    }

    LOG_ASSERT(false);
    return TileOrientation::None;
}

glm::ivec3 TileOrientationToIVec3(TileOrientation orientation) {
    switch (orientation) {
        case TileOrientation::North:
            return {0, 0, 1};
        case TileOrientation::West:
            return {-1, 0, 0};
        case TileOrientation::South:
            return {0, 0, -1};
        case TileOrientation::East:
            return {1, 0, 0};
    }

    LOG_ASSERT(false);
    return {};
}

bool IsSolidBlock(TileType type) {
    return type == TileType::Block || type == TileType::CorridorBlock || type == TileType::StairsBlock || type == TileType::StairsBlock2;
}

bool CanBeOverridenByCorridor(TileType type) {
    return type == TileType::CorridorBlock || type == TileType::StairsBlock || type == TileType::StairsBlock2 || type == TileType::Void;
}

bool IsStairs(TileType type) {
    return type == TileType::StairsAir || type == TileType::StairsBlock || type == TileType::StairsBlock2 || type == TileType::StairsBottomPart ||
           type == TileType::StairsTopPart;
}

bool CanBeAboveOrBelowStairs(TileType type) {
    return CanBeOverridenByCorridor(type) || type == TileType::Block;
}
