#pragma once

#include <memory>

#include <glm/glm.hpp>

#include "WorldGrid.h"
#include "../Utility/Random.h"
#include "../Assert.h"

// box struct and helper functions

struct Box {
    glm::ivec3 offset;
    Dimensions size;
};

bool BoxFitsIntoBox(const Box& box1, const Box& box2);
bool BoxesIntersect(const Box& box1, const Box& box2);

bool PointInsideBox(const glm::ivec3& coords, const Box& box);

// IRoom interface

class IRoom {
 public:
    IRoom() = default;
    virtual ~IRoom() = default;

    virtual void Generate(RNG& rng, SeedType seed) = 0;

    const std::vector<glm::ivec3>& GetEdgeTiles() const;
    void Place(TilesVec& dungeon) const;

 public:
    glm::ivec3 offset;
    Dimensions size;
    TilesVec tiles;

 protected:
    std::vector<glm::ivec3> edgeTiles;
};

// Room helper functions

using Room = std::shared_ptr<IRoom>;

bool RoomsIntersect(const Room& r1, const Room& r2);

glm::vec3 RoomCenter(const Room& room);
glm::ivec3 RoomCenterCoords(const Room& room);

// different rooms

class RectRoom : public IRoom {
 public:
    RectRoom() = default;
    ~RectRoom() override = default;

    void Generate(RNG& rng, SeedType seed) override;
};

class OvalRoom : public IRoom {
 public:
    OvalRoom() = default;
    ~OvalRoom() override = default;

    void Generate(RNG& rng, SeedType seed) override;
};
