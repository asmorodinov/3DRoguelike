#pragma once

#include <memory>

#include "WorldGrid.h"
#include "../Utility/Random.h"
#include "../Assert.h"

struct Box {
    Coordinates offset;
    Dimensions size;
};

bool BoxFitsIntoBox(const Box& box1, const Box& box2);
bool BoxesIntersect(const Box& box1, const Box& box2);

class IRoom {
 public:
    IRoom() = default;
    virtual ~IRoom() = default;

    virtual void Generate(RNG& rng, Seed seed) = 0;
    virtual void Place(TilesVec& dungeon);

 public:
    Coordinates offset;
    Dimensions size;
    TilesVec tiles;
};

using Room = std::shared_ptr<IRoom>;

bool RoomsIntersect(const Room& r1, const Room& r2);

class RectRoom : public IRoom {
 public:
    RectRoom() = default;
    ~RectRoom() override = default;

    void Generate(RNG& rng, Seed seed) override;
};