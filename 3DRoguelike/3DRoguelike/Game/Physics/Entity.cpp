#include "Entity.h"

Box3D Entity::GetCollider() const {
    return Box3D{position - glm::vec3(width / 2, height / 2, width / 2), position + glm::vec3(width / 2, height / 2, width / 2)};
}

void Entity::Jump(float jumpHeight_) {
    if (!grounded) return;

    if (jumpHeight_ == 0.0f) {
        jumpHeight_ = jumpHeight;
    }
    velocity.y = glm::sqrt(-2 * GRAVITY_ACCELERATION.y * jumpHeight_);
}

glm::vec3 Entity::GetFriction() const {
    if (flying) return DRAG_FLY;
    if (grounded) return FRICTION;
    if (velocity.y > 0) return DRAG_JUMP;
    return DRAG_FALL;
}

void Entity::Update(const TilesVec& world, float deltaTime, bool disableCollision) {
    velocity += acceleration * GetFriction() * deltaTime;
    acceleration = glm::vec3();
    grounded = false;

    for (int n = 0; n < 3 && !disableCollision; ++n) {
        auto collider = GetCollider();

        auto vel = velocity * deltaTime;

        // collision variables
        auto collided = false;
        auto collisionInfo = CollisionInfo();

        // check collision with blocks inside certain area around Entity
        auto intPosition = glm::ivec3(glm::round(position));
        auto newPosition = glm::ivec3(glm::round(position + vel));
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

        collisionInfo.entryTime -= 0.001f;

        for (int i = 0; i < velocity.length(); ++i) {
            if (collisionInfo.surfaceNormal[i] != 0.0f) {
                velocity[i] = 0.0f;
                position[i] += vel[i] * collisionInfo.entryTime;
            }
        }

        if (collisionInfo.surfaceNormal.y > 0.5f) {
            grounded = true;
        }
    }

    position += velocity * deltaTime;

    // apply gravity
    auto gravity = flying ? FLYING_ACCELERATION : GRAVITY_ACCELERATION;
    velocity += gravity * deltaTime;

    // apply friction
    for (int i = 0; i < velocity.length(); ++i) {
        auto value = velocity[i] * GetFriction()[i] * deltaTime;

        if (glm::abs(value) > glm::abs(velocity[i])) {
            velocity[i] = 0.0f;
        } else {
            velocity[i] -= value;
        }
    }
}

// member variables setters and getters

void Entity::SetPosition(const glm::vec3& pos) {
    position = pos;
}
glm::vec3 Entity::GetPosition() const {
    return position;
}
void Entity::SetVelocity(const glm::vec3& vel) {
    velocity = vel;
}
glm::vec3 Entity::GetVelocity() const {
    return velocity;
}
void Entity::SetAcceleration(const glm::vec3& acc) {
    acceleration = acc;
}
glm::vec3 Entity::GetAcceleration() const {
    return acceleration;
}

glm::vec3& Entity::GetAcceleration() {
    return acceleration;
}

void Entity::SetFlying(bool flying_) {
    flying = flying_;
}

bool Entity::IsFlying() const {
    return flying;
}
