#pragma once

#include <variant>
#include <glm/glm.hpp>

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

struct MovingObject {
    glm::vec3 position;
    glm::vec3 lastPosition;
};

void ResolveCollision(const CollisionInfo& info, MovingObject& obj);
