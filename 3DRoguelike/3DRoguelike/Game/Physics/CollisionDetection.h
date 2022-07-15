#pragma once

#include <array>
#include <optional>
#include <vector>

#include <glm/glm.hpp>

#include "../Model/Model.h"

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

// collision detection

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

struct Triangle {
    glm::vec3 p0;
    glm::vec3 p1;
    glm::vec3 p2;
};

CollisionInfo SphereVsTriangle(const Sphere& s, const Triangle& t);

std::vector<CollisionInfo> SphereVsModel(const Sphere& s, const ModelData& m);

struct Ray {
    glm::vec3 origin;
    glm::vec3 direction;
};

struct RayIntersectionInfo {
    glm::vec3 intersectionPoint;
    glm::vec3 surfaceNormal;
    Time t;
};
using RayIntersectionResult = std::optional<RayIntersectionInfo>;

RayIntersectionResult RayVsTriangle(const Ray& r, const Triangle& tr);
RayIntersectionResult RayVsModel(const Ray& r, const ModelData& m);

// collision resolution

struct MovingObject {
    glm::vec3 position = glm::vec3();
    glm::vec3 velocity = glm::vec3();
    bool grounded = false;
    int jumpsLeft = 0;
    int maxJumps = 3;
};

void ResolveCollision(const CollisionInfo& info, MovingObject& obj);

void ResolveCollision(const std::vector<CollisionInfo>& infos, MovingObject& obj);
