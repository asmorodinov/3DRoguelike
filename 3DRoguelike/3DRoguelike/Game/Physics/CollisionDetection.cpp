#include "CollisionDetection.h"

#include <glm/gtx/component_wise.hpp>

#include <array>
#include <algorithm>
#include <limits>

ContinousCollisionResult SweptAABB(const Box3D& b1, const glm::vec3& v1, const Box3D& b2) {
    auto time = [](float distance, float velocity) {
        if (velocity != 0.0f) {
            return distance / velocity;
        }
        // velocity == 0.0f
        // not sure why signs are as they are (and whether they actually do matter or not)
        if (distance > 0) {
            return -std::numeric_limits<float>::infinity();
        } else {
            return std::numeric_limits<float>::infinity();
        }
    };

    auto fEntry = [&time](float b1_p1, float b1_p2, float b2_p1, float b2_p2, float vel) {
        return time((vel > 0) ? (b2_p1 - b1_p2) : (b2_p2 - b1_p1), vel);
    };
    auto fExit = [&time](float b1_p1, float b1_p2, float b2_p1, float b2_p2, float vel) {
        return time((vel > 0) ? (b2_p2 - b1_p1) : (b2_p1 - b1_p2), vel);
    };

    auto vEntry = glm::vec3(fEntry(b1.p1.x, b1.p2.x, b2.p1.x, b2.p2.x, v1.x), fEntry(b1.p1.y, b1.p2.y, b2.p1.y, b2.p2.y, v1.y),
                            fEntry(b1.p1.z, b1.p2.z, b2.p1.z, b2.p2.z, v1.z));
    auto vExit = glm::vec3(fExit(b1.p1.x, b1.p2.x, b2.p1.x, b2.p2.x, v1.x), fExit(b1.p1.y, b1.p2.y, b2.p1.y, b2.p2.y, v1.y),
                           fExit(b1.p1.z, b1.p2.z, b2.p1.z, b2.p2.z, v1.z));

    // no collision
    if (glm::all(glm::lessThan(vEntry, glm::vec3(0)))) {
        return std::nullopt;
    }
    // no collision
    if (glm::any(glm::greaterThan(vEntry, glm::vec3(1)))) {
        return std::nullopt;
    }

    auto entry = glm::compMax(vEntry);
    auto exit = glm::compMin(vExit);

    // no collision
    if (entry > exit) {
        return std::nullopt;
    }

    auto normal = glm::vec3(0.0f);
    if (vEntry.x == entry) normal.x = (v1.x > 0) ? -1.0f : 1.0f;
    if (vEntry.y == entry) normal.y = (v1.y > 0) ? -1.0f : 1.0f;
    if (vEntry.z == entry) normal.z = (v1.z > 0) ? -1.0f : 1.0f;

    return ContinousCollisionInfo{entry, normal};
}

// sphere vs cube

bool quickSphereVsCube(const Sphere& s, const Cube& cube) {
    auto c = cube.center;
    auto r = glm::sqrt(3.0f) * cube.sideLength / 2.0f;

    return glm::distance(c, s.center) <= (r + s.radius);
}

bool pointInsideCube(const glm::vec3& point, const Cube& cube) {
    return glm::all(glm::lessThanEqual(cube.center - cube.sideLength / 2, point)) &&
           glm::all(glm::lessThanEqual(point, cube.center + cube.sideLength / 2));
}

CollisionInfo SphereVsCube(const Sphere& s, const Cube& cube) {
    static constexpr auto eps = 0.00000001f;

    if (!quickSphereVsCube(s, cube)) return CollisionInfo();

    auto minCorner = cube.center - glm::vec3(cube.sideLength / 2);
    auto maxCorner = cube.center + glm::vec3(cube.sideLength / 2);

    // center of sphere is inside of the cube

    if (pointInsideCube(s.center, cube)) {
        auto points = std::array<glm::vec3, 6>{glm::vec3(minCorner.x, s.center.y, s.center.z), glm::vec3(maxCorner.x, s.center.y, s.center.z),
                                               glm::vec3(s.center.x, minCorner.y, s.center.z), glm::vec3(s.center.x, maxCorner.y, s.center.z),
                                               glm::vec3(s.center.x, s.center.y, minCorner.z), glm::vec3(s.center.x, s.center.y, maxCorner.z)};

        auto nearestPoint = *std::min_element(points.begin(), points.end(), [&s](const glm::vec3& v1, const glm::vec3& v2) {
            return glm::distance(v1, s.center) < glm::distance(v2, s.center);
        });

        auto rayToNearest = nearestPoint - s.center;
        auto rayLength = glm::length(rayToNearest);
        auto overlap = s.radius + rayLength;
        auto rayNormalized = (rayLength <= eps) ? glm::vec3(0.0f) : rayToNearest / rayLength;

        return CollisionInfo{true, overlap, rayNormalized};
    }

    // center of sphere is outside of the cube

    auto nearestPoint = glm::clamp(s.center, minCorner, maxCorner);
    auto rayToNearest = nearestPoint - s.center;
    auto rayLength = glm::length(rayToNearest);
    auto overlap = s.radius - rayLength;

    if (overlap < 0.0f) return CollisionInfo();

    auto rayNormalized = (rayLength <= eps) ? glm::vec3(0.0f) : rayToNearest / rayLength;

    return CollisionInfo{true, overlap, -rayNormalized};
}

void ResolveCollision(const CollisionInfo& info, MovingObject& obj) {
    static constexpr auto eps = 0.00000001f;
    static constexpr auto eps2 = 0.0001f;

    if (!info.isColliding) return;

    const auto& normal = info.penetrationNormal;
    const auto depth = info.penetrationDepth;

    auto velocityLength = glm::length(obj.velocity);
    auto velocityNormalized = (velocityLength <= eps) ? glm::vec3(0.0f) : obj.velocity / velocityLength;

    auto undesiredMotion = normal * glm::dot(velocityNormalized, normal);
    auto desiredMotion = velocityNormalized - undesiredMotion;

    obj.position += normal * (depth + eps2);        // remove penetration
    obj.velocity = desiredMotion * velocityLength;  // update velocity

    // handle collision with ground
    if (glm::dot(normal, glm::vec3(0.0f, 1.0f, 0.0f)) > 0.3f) {
        obj.grounded = true;
    }
}
