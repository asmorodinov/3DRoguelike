#pragma once

#include <optional>

#include <glm/glm.hpp>

using Time = float;

struct CollisionInfo {
    Time entryTime = 0.0f;
    glm::vec3 surfaceNormal = glm::vec3();
};

// std::nullopt means there was no collision
using CollisionResult = std::optional<CollisionInfo>;

struct Box3D {
    glm::vec3 p1;  // corner with minimum vertex coords
    glm::vec3 p2;  // corner with maximum vertex coords
};

// b1 is moving with velocity v1, b2 - static collider
CollisionResult SweptAABB(const Box3D& b1, const glm::vec3& v1, const Box3D& b2);
