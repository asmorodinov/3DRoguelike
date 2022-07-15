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

bool quickSphereVsModel(const Sphere& s, const ModelData& m) {
    auto c = glm::vec3();
    for (const auto& face : m) {
        for (const auto& vertex : face) {
            c += vertex.position;
        }
    }
    c *= 1.0f / (3 * m.size());

    auto r = 0.0f;
    for (const auto& face : m) {
        for (const auto& vertex : face) {
            r = glm::max(r, glm::distance(c, vertex.position));
        }
    }

    return glm::distance(c, s.center) <= (r + s.radius);
}

std::vector<CollisionInfo> SphereVsModel(const Sphere& s, const ModelData& m) {
    if (!quickSphereVsModel(s, m)) {
        return {};
    }

    auto res = std::vector<CollisionInfo>();

    for (const auto& face : m) {
        res.push_back(SphereVsTriangle(s, Triangle{face[0].position, face[1].position, face[2].position}));
    }

    return res;
}

void ResolveCollision(const CollisionInfo& info, MovingObject& obj) {
    static constexpr auto eps = 0.00000001f;
    static constexpr auto eps2 = 0.001f;

    if (!info.isColliding) return;

    auto normal = info.penetrationNormal;
    const auto depth = info.penetrationDepth;

    // handle collision with ground
    auto up = glm::vec3(0.0f, 1.0f, 0.0f);
    if (glm::dot(normal, up) > 0.8f) {
        obj.grounded = true;
        obj.jumpsLeft = obj.maxJumps;

        normal = up;  // change normal so that player won't slide down when standing still on a sloped surface
    }

    auto velocityLength = glm::length(obj.velocity);
    auto velocityNormalized = (velocityLength <= eps) ? glm::vec3(0.0f) : obj.velocity / velocityLength;

    auto cos = glm::dot(velocityNormalized, normal);
    auto undesiredMotion = (cos > 0.0f) ? glm::vec3(0.0f) : normal * cos;  // if cos > 0 it means that we are already moving away from surface
    auto desiredMotion = velocityNormalized - undesiredMotion;

    obj.position += normal * (depth + eps2);        // remove penetration
    obj.velocity = desiredMotion * velocityLength;  // update velocity
}

void ResolveCollision(const std::vector<CollisionInfo>& infos, MovingObject& obj) {
    for (const auto& info : infos) {
        ResolveCollision(info, obj);
    }
}
