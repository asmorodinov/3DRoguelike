#pragma once

#include <memory>

#include <glm/glm.hpp>

#include "WorldGrid.h"
#include "../Utility/Random.h"
#include "../Assert.h"

struct Box {
    Coordinates offset;
    Dimensions size;
};

bool BoxFitsIntoBox(const Box& box1, const Box& box2);
bool BoxesIntersect(const Box& box1, const Box& box2);

bool PointInsideBox(const Coordinates& coords, const Box& box);

class IRoom {
 public:
    IRoom() = default;
    virtual ~IRoom() = default;

    virtual void Generate(RNG& rng, SeedType seed) = 0;
    virtual std::vector<Coordinates> GetEdgeTiles() = 0;
    virtual void Place(TilesVec& dungeon);

 public:
    Coordinates offset;
    Dimensions size;
    TilesVec tiles;
};

using Room = std::shared_ptr<IRoom>;

bool RoomsIntersect(const Room& r1, const Room& r2);
glm::vec3 RoomCenter(const Room& room);
Coordinates RoomCenterCoords(const Room& room);

class RectRoom : public IRoom {
 public:
    RectRoom() = default;
    ~RectRoom() override = default;

    void Generate(RNG& rng, SeedType seed) override;
    std::vector<Coordinates> GetEdgeTiles() override;

 private:
    std::vector<Coordinates> edgeTiles;
};