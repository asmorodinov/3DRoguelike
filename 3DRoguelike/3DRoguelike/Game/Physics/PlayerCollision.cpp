#include "PlayerCollision.h"

#include "CollisionDetection.h"

void ResolveCollisionWithWorld(Camera& camera, const TilesVec& world) {
    auto position = camera.Position;
    position = glm::round(position);
    auto x = static_cast<size_t>(static_cast<int>(position.x));
    auto y = static_cast<size_t>(static_cast<int>(position.y));
    auto z = static_cast<size_t>(static_cast<int>(position.z));

    const auto& dimensions = world.GetDimensions();
    for (size_t i = x - 1; i <= x + 1; ++i) {
        for (size_t j = y - 1; j <= y + 1; ++j) {
            for (size_t k = z - 1; k <= z + 1; ++k) {
                auto coords = Coordinates{i, j, k};
                if (!coords.IsInBounds(dimensions)) continue;

                const auto& tile = world.Get(coords);
                if (tile.type != TileType::Block && tile.type != TileType::CorridorBlock) continue;

                auto info = SphereVsSphere(Sphere{camera.Position, 0.5f}, Sphere{coords.AsVec3(), 0.5f});
                if (!info.isColliding) continue;

                auto object = MovingObject{camera.Position, camera.LastPosition};
                ResolveCollision(info, object);
                camera.Position = object.position;
                camera.LastPosition = object.lastPosition;
            }
        }
    }
}
