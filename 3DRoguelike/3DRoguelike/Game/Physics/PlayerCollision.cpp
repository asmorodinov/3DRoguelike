#include "PlayerCollision.h"

#include "CollisionDetection.h"

void resolveCollision(MovingObject& object, const TilesVec& world) {
    auto position = object.position;
    position = glm::round(position);
    auto x = static_cast<int>(position.x);
    auto y = static_cast<int>(position.y);
    auto z = static_cast<int>(position.z);

    const auto& dimensions = world.GetDimensions();
    for (int i = x - 1; i <= x + 1; ++i) {
        for (int j = y - 1; j <= y + 1; ++j) {
            for (int k = z - 1; k <= z + 1; ++k) {
                auto coords = Coordinates{size_t(i), size_t(j), size_t(k)};
                if (!coords.IsInBounds(dimensions)) continue;

                const auto& tile = world.Get(coords);
                if (tile.type != TileType::Block && tile.type != TileType::CorridorBlock) continue;

                auto faces = std::array<bool, 6>();
                faces[0] = IsAir(coords + Coordinates{0, 0, size_t(-1)}, world);
                faces[1] = IsAir(coords + Coordinates{0, 0, size_t(1)}, world);
                faces[2] = IsAir(coords + Coordinates{size_t(-1), 0, 0}, world);
                faces[3] = IsAir(coords + Coordinates{size_t(1), 0, 0}, world);
                faces[4] = IsAir(coords + Coordinates{0, size_t(-1), 0}, world);
                faces[5] = IsAir(coords + Coordinates{0, size_t(1), 0}, world);

                auto info = SphereVsCube(Sphere{object.position, 0.3f}, Cube{coords.AsVec3(), 1.0f}, faces);
                ResolveCollision(info, object);
            }
        }
    }
}

void ResolveCollisionWithWorld(Camera& camera, const TilesVec& world, float dt) {
    auto object = MovingObject{camera.Position, camera.Velocity};

    int ccd_max = 50;
    object.velocity *= 1.0f / ccd_max;

    for (int ccd = 0; ccd < ccd_max; ++ccd) {
        resolveCollision(object, world);

        auto step = object.velocity * dt;
        object.position += step;
    }

    camera.Position = object.position;
}
