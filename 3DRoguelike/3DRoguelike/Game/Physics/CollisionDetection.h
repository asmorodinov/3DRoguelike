#pragma once

#include <optional>

#include <glm/glm.hpp>

// continous collision detection

using Time = float;

struct ContinousCollisionInfo {
    Time entryTime = 0.0f;
    glm::vec3 surfaceNormal = glm::vec3();
};

// std::nullopt means there was no collision
using ContinousCollisionResult = std::optional<ContinousCollisionInfo>;

struct Box3D {
    glm::vec3 p1;  // corner with minimum vertex coords
    glm::vec3 p2;  // corner with maximum vertex coords
};

// b1 is moving with velocity v1, b2 - static collider
ContinousCollisionResult SweptAABB(const Box3D& b1, const glm::vec3& v1, const Box3D& b2);

// discrete collision detection

using Depth = float;
using Radius = float;
using Length = float;

struct Sphere {
    glm::vec3 center;
    Radius radius;
};

struct Cube {
    glm::vec3 center;
    Length sideLength;
};

struct CollisionInfo {
    bool isColliding = false;
    Depth penetrationDepth = 0.0f;
    glm::vec3 penetrationNormal = glm::vec3();
};

CollisionInfo SphereVsCube(const Sphere& s, const Cube& c);

struct MovingObject {
    glm::vec3 position;
    glm::vec3 velocity;
    bool grounded;
    int jumpsLeft;
    int maxJumps;
};

void ResolveCollision(const CollisionInfo& info, MovingObject& obj);