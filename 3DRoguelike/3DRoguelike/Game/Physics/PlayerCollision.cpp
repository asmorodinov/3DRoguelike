#include "PlayerCollision.h"

#include "CollisionDetection.h"
#include "../Utility/LogDuration.h"

void resolveCollision(MovingObject& object, const TilesVec& world) {
    auto position = glm::ivec3(glm::round(object.position));
    auto newPosition = glm::ivec3(glm::round(object.position + object.velocity));
    auto min = glm::min(position, newPosition);
    auto max = glm::max(position, newPosition);

    object.position += object.velocity;

    const auto& dimensions = world.GetDimensions();
    for (int i = min.x - 1; i <= max.x + 1; ++i) {
        for (int j = min.y - 1; j <= max.y + 1; ++j) {
            for (int k = min.z - 1; k <= max.z + 1; ++k) {
                auto coords = Coordinates{size_t(i), size_t(j), size_t(k)};
                auto intCoords = glm::ivec3(i, j, k);

                const auto& tile = world.GetInOrOutOfBounds(intCoords);
                if (tile.type != TileType::Block && tile.type != TileType::CorridorBlock) continue;

                auto info = SphereVsCube(Sphere{object.position, 0.25f}, Cube{glm::vec3(intCoords), 1.0f});
                ResolveCollision(info, object);
            }
        }
    }
}

void ResolveCollisionWithWorld(Camera& camera, const TilesVec& world, float dt, bool disableCollision) {
    if (disableCollision) {
        camera.Position += 5.0f * camera.Velocity * dt;
        return;
    }

    auto n = 200;
    auto object = MovingObject{camera.Position, camera.Velocity * dt * (1.0f / n)};
    for (int i = 0; i < n; ++i) {
        resolveCollision(object, world);
    }

    camera.Position = object.position;
}
