#include "PlayerCollision.h"

#include "../Model/Model.h"

// continous collision detection

void ResolveCollisionWithWorldContinous(const Box3D& boxCollider, MovingObject& object, const TilesVec& world, float deltaTime,
                                        bool disableCollision) {
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

        if (glm::dot(collisionInfo.surfaceNormal, glm::vec3(0.0f, 1.0f, 0.0f)) > 0.3f) {
            object.grounded = true;
            object.jumpsLeft = object.maxJumps;
        }
    }

    // update position
    object.position += object.velocity * deltaTime;
}

// discrete collision detection

void resolvePlayerVsWorldCollision(const Sphere& sphereCollider, MovingObject& object, const TilesVec& world, float deltaTime) {
    auto intPosition = glm::ivec3(glm::round(object.position));
    auto newPosition = glm::ivec3(glm::round(object.position + object.velocity * deltaTime));
    auto min = glm::min(intPosition, newPosition);
    auto max = glm::max(intPosition, newPosition);
    const auto& dimensions = world.GetDimensions();

    for (int i = min.x - 1; i <= max.x + 1; ++i) {
        for (int j = min.y - 1; j <= max.y + 1; ++j) {
            for (int k = min.z - 1; k <= max.z + 1; ++k) {
                auto coords = Coordinates{size_t(i), size_t(j), size_t(k)};
                auto intCoords = glm::ivec3(i, j, k);

                const auto& tile = world.GetInOrOutOfBounds(intCoords);
                if (tile.type != TileType::Block && tile.type != TileType::CorridorBlock && tile.type != TileType::StairsTopBlock &&
                    tile.type != TileType::StairsBottomBlock)
                    continue;

                // collision with blocks

                if (tile.type == TileType::Block || tile.type == TileType::CorridorBlock) {
                    // check for collision
                    auto info = SphereVsCube(Sphere{object.position, sphereCollider.radius}, Cube{glm::vec3(intCoords), 1.0f});
                    // collision response
                    ResolveCollision(info, object);

                    continue;
                }

                // collision with stairs

                auto center = glm::vec3(intCoords);

                if (tile.type == TileType::StairsBottomBlock) {
                    center -= TileOrientationToIVec3(tile.orientation);
                }

                auto model = GetSlopeModelData(0);
                if (tile.orientation == TileOrientation::West) {
                    model = GetSlopeModelData(1);
                } else if (tile.orientation == TileOrientation::South) {
                    model = GetSlopeModelData(2);
                } else if (tile.orientation == TileOrientation::East) {
                    model = GetSlopeModelData(3);
                }
                Move(model, center);

                // check for collision
                auto info = SphereVsModel(Sphere{object.position, sphereCollider.radius}, model);
                // collision response
                ResolveCollision(info, object);
            }
        }
    }

    // update position
    object.position += object.velocity * deltaTime;
}

void ResolveCollisionWithWorldDiscrete(const Sphere& sphereCollider, MovingObject& object, const TilesVec& world, float deltaTime,
                                       bool disableCollision) {
    if (disableCollision) {
        object.position += object.velocity * deltaTime;
        return;
    }

    // perform multiple discrete substeps per frame

    int steps = 150;
    for (int i = 0; i < steps; ++i) {
        resolvePlayerVsWorldCollision(sphereCollider, object, world, deltaTime / steps);
    }
}

// ray cast

RayIntersectionResult RayCast(const Ray& ray, const TilesVec& world, Length maxLength) {
    auto hit = false;
    auto res = RayIntersectionInfo{glm::vec3(), glm::vec3(), std::numeric_limits<float>::infinity()};

    auto intPosition = glm::ivec3(glm::round(ray.origin));
    auto newPosition = glm::ivec3(glm::round(ray.origin + ray.direction * maxLength));
    auto min = glm::min(intPosition, newPosition);
    auto max = glm::max(intPosition, newPosition);
    const auto& dimensions = world.GetDimensions();

    for (int i = min.x - 1; i <= max.x + 1; ++i) {
        for (int j = min.y - 1; j <= max.y + 1; ++j) {
            for (int k = min.z - 1; k <= max.z + 1; ++k) {
                auto coords = Coordinates{size_t(i), size_t(j), size_t(k)};
                auto intCoords = glm::ivec3(i, j, k);

                const auto& tile = world.GetInOrOutOfBounds(intCoords);
                if (tile.type != TileType::Block && tile.type != TileType::CorridorBlock && tile.type != TileType::StairsTopBlock &&
                    tile.type != TileType::StairsBottomBlock)
                    continue;

                auto center = glm::vec3(intCoords);

                if (tile.type == TileType::StairsBottomBlock) {
                    center -= TileOrientationToIVec3(tile.orientation);
                }

                auto model = ModelData{};
                if (tile.type == TileType::StairsTopBlock || tile.type == TileType::StairsBottomBlock) {
                    if (tile.orientation == TileOrientation::North)
                        model = GetSlopeModelData(0);
                    else if (tile.orientation == TileOrientation::West)
                        model = GetSlopeModelData(1);
                    else if (tile.orientation == TileOrientation::South)
                        model = GetSlopeModelData(2);
                    else if (tile.orientation == TileOrientation::East)
                        model = GetSlopeModelData(3);
                } else if (tile.type == TileType::Block || tile.type == TileType::CorridorBlock) {
                    model = GetCubeModelData();
                }
                Move(model, center);

                // check for collision
                auto intersection = RayVsModel(ray, model);

                if (!intersection.has_value()) continue;
                hit = true;

                const auto& value = intersection.value();
                if (value.t < res.t) {
                    res = value;
                }
            }
        }
    }

    if (!hit) return std::nullopt;
    if (res.t > maxLength) return std::nullopt;

    return res;
}

RayIntersectionResult CastMultipleRays(const Ray& ray, const std::vector<glm::vec3>& originDeltas, const TilesVec& world, Length maxLength) {
    auto hit = false;
    auto res = RayIntersectionInfo{glm::vec3(), glm::vec3(), std::numeric_limits<float>::infinity()};

    for (const auto& delta : originDeltas) {
        auto intersection = RayCast(Ray{ray.origin + delta, ray.direction}, world, maxLength);

        if (!intersection.has_value()) continue;

        const auto& value = intersection.value();
        if (!hit) {
            hit = true;
            res = value;
            continue;
        }

        if (glm::distance(value.intersectionPoint, ray.origin) < glm::distance(res.intersectionPoint, ray.origin)) {
            res = value;
        }
    }

    if (!hit) return std::nullopt;

    return res;
}
