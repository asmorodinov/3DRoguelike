#include "PlayerCollision.h"

void ResolveCollisionWithWorld(const Box3D& boxCollider, MovingObject& object, const TilesVec& world, float deltaTime, bool disableCollision) {
    for (int n = 0; n < 3 && !disableCollision; ++n) {
        auto collider = Box3D{object.position + boxCollider.p1, object.position + boxCollider.p2};

        auto vel = object.velocity * deltaTime;

        // collision variables
        auto collided = false;
        auto collisionInfo = ContinousCollisionInfo();

        // check collision with blocks inside certain area around Entity
        auto intPosition = glm::ivec3(glm::round(object.position));
        auto newPosition = glm::ivec3(glm::round(object.position + vel));
        auto min = glm::min(intPosition, newPosition);
        auto max = glm::max(intPosition, newPosition);
        const auto& dimensions = world.GetDimensions();
        for (int i = min.x - 1; i <= max.x + 1; ++i) {
            for (int j = min.y - 1; j <= max.y + 1; ++j) {
                for (int k = min.z - 1; k <= max.z + 1; ++k) {
                    auto coords = Coordinates{size_t(i), size_t(j), size_t(k)};
                    auto intCoords = glm::ivec3(i, j, k);

                    const auto& tile = world.GetInOrOutOfBounds(intCoords);
                    if (tile.type != TileType::Block && tile.type != TileType::CorridorBlock) continue;

                    // check for collision
                    auto collisionResult = SweptAABB(collider, vel, Box3D{glm::vec3(intCoords) - 0.5f, glm::vec3(intCoords) + 0.5f});
                    if (!collisionResult.has_value()) continue;
                    const auto& info = collisionResult.value();

                    // update collision info
                    if (!collided || info.entryTime < collisionInfo.entryTime) {
                        collided = true;
                        collisionInfo = info;
                    }
                }
            }
        }

        if (!collided) break;

        // update velocity and position

        collisionInfo.entryTime -= 0.01f;

        for (int i = 0; i < object.velocity.length(); ++i) {
            if (collisionInfo.surfaceNormal[i] != 0.0f) {
                object.velocity[i] = 0.0f;
                object.position[i] += vel[i] * collisionInfo.entryTime;
            }
        }

        if (collisionInfo.surfaceNormal.y > 0.5f) {
            object.grounded = true;
            object.jumpsLeft = object.maxJumps;
        }
    }
}
