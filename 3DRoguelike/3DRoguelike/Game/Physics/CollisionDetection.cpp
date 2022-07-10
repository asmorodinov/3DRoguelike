#include "CollisionDetection.h"

#include <glm/gtx/component_wise.hpp>

#include <limits>

CollisionResult SweptAABB(const Box3D& b1, const glm::vec3& v1, const Box3D& b2) {
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
    if (glm::any(glm::greaterThan(vExit, glm::vec3(1)))) {
        return std::nullopt;
    }

    auto entry = glm::compMax(vEntry);
    auto exit = glm::compMin(vExit);

    // no collision
    if (entry > exit) {
        return std::nullopt;
    }

    auto normal = glm::vec3(0.0f);
    if (vEntry.x == entry) normal.x = (v1.x > 0) ? -1 : 1;
    if (vEntry.y == entry) normal.y = (v1.y > 0) ? -1 : 1;
    if (vEntry.z == entry) normal.z = (v1.z > 0) ? -1 : 1;

    return CollisionInfo{entry, normal};
}
