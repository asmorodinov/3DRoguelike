#include "CollisionDetection.h"

// sphere vs sphere

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

// sphere vs triangle

bool quickSphereVsTriangle(const Sphere& s, const Triangle& t) {
    auto c = (t.p0 + t.p1 + t.p2) / 3.0f;
    auto r = glm::max(glm::distance(c, t.p0), glm::max(glm::distance(c, t.p1), glm::distance(c, t.p2)));

    return glm::distance(c, s.center) <= (r + s.radius);
}

glm::vec3 ClosestPointOnLineSegment(const glm::vec3& A, const glm::vec3& B, const glm::vec3& Point) {
    auto AB = B - A;
    float t = glm::dot(Point - A, AB) / glm::dot(AB, AB);
    return A + glm::clamp(t, 0.0f, 1.0f) * AB;
}

CollisionInfo SphereVsTriangle(const Sphere& s, const Triangle& t) {
    if (!quickSphereVsTriangle(s, t)) return CollisionInfo();

    auto N = glm::normalize(glm::cross(t.p1 - t.p0, t.p2 - t.p0));  // triangle plane normal
    auto dist = glm::dot(s.center - t.p0, N);                       // signed distance between sphere center and triangle plane

    if (dist < -s.radius || dist > s.radius) return CollisionInfo();  // sphere does not intersect triangle plane

    auto point0 = s.center - N * dist;  // projected sphere center on triangle plane

    auto c0 = glm::cross(point0 - t.p0, t.p1 - t.p0);
    auto c1 = glm::cross(point0 - t.p1, t.p2 - t.p1);
    auto c2 = glm::cross(point0 - t.p2, t.p0 - t.p2);

    auto inside = glm::dot(c0, N) <= 0 && glm::dot(c1, N) <= 0 && glm::dot(c2, N) <= 0;
    auto intersection_vec = glm::vec3();

    if (inside) {
        intersection_vec = s.center - point0;
    } else {
        auto radiussq = s.radius * s.radius;

        // Edge 1:
        auto point1 = ClosestPointOnLineSegment(t.p0, t.p1, s.center);
        auto v1 = s.center - point1;
        auto distsq1 = glm::dot(v1, v1);
        auto intersects = distsq1 < radiussq;

        // Edge 2:
        auto point2 = ClosestPointOnLineSegment(t.p1, t.p2, s.center);
        auto v2 = s.center - point2;
        auto distsq2 = glm::dot(v2, v2);
        intersects |= distsq2 < radiussq;

        // Edge 3:
        auto point3 = ClosestPointOnLineSegment(t.p2, t.p0, s.center);
        auto v3 = s.center - point3;
        auto distsq3 = glm::dot(v3, v3);
        intersects |= distsq3 < radiussq;

        if (!intersects) return CollisionInfo();

        auto best_distsq = distsq1;
        auto best_point = point1;
        intersection_vec = v1;

        if (distsq2 < best_distsq) {
            best_distsq = distsq2;
            best_point = point2;
            intersection_vec = v2;
        }
        if (distsq3 < best_distsq) {
            best_distsq = distsq3;
            best_point = point3;
            intersection_vec = v3;
        }
    }

    auto len = glm::length(intersection_vec);          // vector3 length calculation: sqrt(dot(v, v))
    auto penetration_normal = intersection_vec / len;  // normalize
    auto penetration_depth = s.radius - len;           // radius = sphere radius

    return CollisionInfo{true, penetration_depth, penetration_normal};
}

// collision resolution

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
