#pragma once

#include "Tile.h"
#include "WorldGrid.h"
#include "TileRenderer.h"
#include "Room.h"
#include "../Utility/Random.h"

class Dungeon {
 public:
    Dungeon(const Dimensions& dimensions_, Seed seed_ = Seed());

    void SetSeed(Seed seed_);
    Seed GetSeed() const;

    void Generate();
    void Render();

 private:
    void placeRooms();
    void placeCorridors();
    void reset();

 private:
    Dimensions dimensions;
    Seed seed;
    RNG rng;
    TilesVec tiles;
    std::vector<Room> rooms;
    TileRenderer renderer;
};