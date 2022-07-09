#pragma once

#include <optional>
#include <array>
#include <glm/glm.hpp>

using Length = float;
using Radius = float;
using Depth = float;

struct Sphere {
    glm::vec3 center;
    Radius radius;
};

struct CollisionInfo {
    bool isColliding = false;
    Depth penetrationDepth = 0.0f;
    glm::vec3 penetrationNormal = glm::vec3();
};

CollisionInfo SphereVsSphere(const Sphere& s1, const Sphere& s2);

struct Triangle {
    glm::vec3 p0;
    glm::vec3 p1;
    glm::vec3 p2;
};

CollisionInfo SphereVsTriangle(const Sphere& s, const Triangle& t);

struct Cube {
    glm::vec3 center;
    Length sideLength;
};
std::optional<std::array<CollisionInfo, 12>> SphereVsCube(const Sphere& s, const Cube& c);

struct MovingObject {
    glm::vec3 position;
    glm::vec3 lastPosition;
};

void ResolveCollision(const CollisionInfo& info, MovingObject& obj);
void ResolveCollision(const std::optional<std::array<CollisionInfo, 12>>& info, MovingObject& obj);
