#include "PlayerCollision.h"

#include "../Model/Model.h"

// discrete collision detection

void resolveSphereVsWorldCollision(const Sphere& sphereCollider, MovingObject& object, const TilesVec& world, float deltaTime) {
    auto intPosition = glm::ivec3(glm::round(object.position));
    auto newPosition = glm::ivec3(glm::round(object.position + object.velocity * deltaTime));
    auto min = glm::min(intPosition, newPosition);
    auto max = glm::max(intPosition, newPosition);
    const auto& dimensions = world.GetDimensions();

    for (int i = min.x - 1; i <= max.x + 1; ++i) {
        for (int j = min.y - 1; j <= max.y + 1; ++j) {
            for (int k = min.z - 1; k <= max.z + 1; ++k) {
                auto coords = glm::ivec3(i, j, k);

                const auto& tile = world.GetInOrOutOfBounds(coords);
                if (!IsSolidBlock(tile.type) && tile.type != TileType::StairsTopPart && tile.type != TileType::StairsBottomPart) continue;

                // collision with blocks

                if (IsSolidBlock(tile.type)) {
                    // check for collision
                    auto info = SphereVsCube(Sphere{object.position, sphereCollider.radius}, Cube{glm::vec3(coords), 1.0f});
                    // collision response
                    ResolveCollision(info, object);

                    continue;
                }

                // collision with stairs

                auto center = glm::vec3(coords);

                if (tile.type == TileType::StairsBottomPart) {
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

void ResolveSphereVsWorldCollision(const Sphere& sphereCollider, MovingObject& object, const TilesVec& world, float deltaTime,
                                   bool disableCollision) {
    if (disableCollision) {
        object.position += object.velocity * deltaTime;
        return;
    }

    // perform multiple discrete substeps per frame

    int steps = 150;
    for (int i = 0; i < steps; ++i) {
        resolveSphereVsWorldCollision(sphereCollider, object, world, deltaTime / steps);
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
                auto coords = glm::ivec3(i, j, k);

                const auto& tile = world.GetInOrOutOfBounds(coords);
                if (!IsSolidBlock(tile.type) && tile.type != TileType::StairsTopPart && tile.type != TileType::StairsBottomPart) continue;

                auto center = glm::vec3(coords);

                if (tile.type == TileType::StairsBottomPart) {
                    center -= TileOrientationToIVec3(tile.orientation);
                }

                auto model = ModelData{};
                if (tile.type == TileType::StairsTopPart || tile.type == TileType::StairsBottomPart) {
                    if (tile.orientation == TileOrientation::North)
                        model = GetSlopeModelData(0);
                    else if (tile.orientation == TileOrientation::West)
                        model = GetSlopeModelData(1);
                    else if (tile.orientation == TileOrientation::South)
                        model = GetSlopeModelData(2);
                    else if (tile.orientation == TileOrientation::East)
                        model = GetSlopeModelData(3);
                } else if (IsSolidBlock(tile.type)) {
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
