#include "CollisionDetection.h"

#include <algorithm>
#include <array>

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

// collision resolution

void ResolveCollision(const CollisionInfo& info, MovingObject& obj) {
    static constexpr auto eps = 0.00005f;

    if (!info.isColliding) {
        return;
    }
    const auto& normal = info.penetrationNormal;
    const auto depth = info.penetrationDepth;

    obj.position += normal * (depth + eps);
}
