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
