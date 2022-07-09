#include "CollisionDetection.h"

// s1 is dynamic and s2 is static
CollisionInfo SphereVsSphere(const Sphere& s1, const Sphere& s2) {
    auto distance = glm::distance(s1.center, s2.center);
    auto radiusSum = s1.radius + s2.radius;

    if (distance >= radiusSum) {
        return CollisionInfo();
    }

    auto depth = radiusSum - distance;
    auto normal = glm::normalize(s1.center - s2.center);

    return CollisionInfo{true, depth, normal};
}

void ResolveCollision(const CollisionInfo& info, MovingObject& obj) {
    if (!info.isColliding) {
        return;
    }
    const auto& normal = info.penetrationNormal;
    const auto depth = info.penetrationDepth;

    auto velocity = obj.position - obj.lastPosition;
    auto velocityLength = glm::length(velocity);
    auto velocityNormalized = velocity / velocityLength;

    auto undesiredMotion = normal * glm::dot(velocityNormalized, normal);  // normalized
    auto desiredMotion = velocityNormalized - undesiredMotion;             // normalized

    auto newPosition = obj.position + normal * (depth + 0.0001f);  // remove penetration

    auto moved = glm::length(newPosition - obj.lastPosition);
    if (moved < velocityLength) {
        newPosition += (velocityLength - moved) * desiredMotion;  // also move a little bit in the desired direction
    }

    obj.position = newPosition;
}
