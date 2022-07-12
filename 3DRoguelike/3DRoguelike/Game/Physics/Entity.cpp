#include "Entity.h"

#include "../Utility/LogDuration.h"
#include "../Physics/PlayerCollision.h"

Box3D Entity::GetCollider() const {
    return Box3D{-glm::vec3(width / 2, height / 2, width / 2), glm::vec3(width / 2, height / 2, width / 2)};
}

void Entity::Jump(float jumpHeight_) {
    if (jumpsLeft == 0) {
        return;
    }
    --jumpsLeft;

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
    // LOG_DURATION("player update");

    velocity += acceleration * GetFriction() * deltaTime;
    acceleration = glm::vec3();
    grounded = false;

    // collision detection
    ResolveCollisionWithWorld(GetCollider(), *this, world, deltaTime, disableCollision);

    // physics calculations
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
