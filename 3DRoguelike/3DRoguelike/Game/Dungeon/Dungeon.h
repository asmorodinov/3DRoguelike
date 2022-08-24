#pragma once

#include "Tile.h"
#include "WorldGrid.h"
#include "TileRenderer.h"
#include "Room.h"
#include "../Utility/Random.h"

class Dungeon {
 public:
    Dungeon(const Dimensions& dimensions_, SeedType seed_ = SeedType());

    void SetSeed(SeedType seed_);
    SeedType GetSeed() const;

    void Generate();
    void Render();

    const TilesVec& GetTiles() const;

    glm::ivec3 GetSpawnPoint() const;

    size_t WhichRoomPointIsInside(const glm::ivec3& coords) const;

 private:
    void placeRooms();
    void placeCorridors();
    void reset();

 private:
    Dimensions dimensions;
    SeedType seed;
    RNG rng;
    TilesVec tiles;
    std::vector<Room> rooms;
    TileRenderer renderer;

    glm::ivec3 spawn;
};
