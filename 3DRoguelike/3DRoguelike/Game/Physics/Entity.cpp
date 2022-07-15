#include "Entity.h"

#include <glm/gtx/quaternion.hpp>

#include "../Utility/LogDuration.h"
#include "../Physics/PlayerCollision.h"

Sphere Entity::GetSphereCollider() const {
    return Sphere{glm::vec3(), radius};
}

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

bool adjustVelocityToSlope(glm::vec3& position, float radius, glm::vec3& velocity, const TilesVec& world) {
    static constexpr auto eps = 0.00000001f;

    auto length = glm::length(velocity);
    if (length <= eps) return false;

    // moving upwards (jumping) => no need to adjust velocity
    if (velocity.y > 0.0f) return false;

    // perform raycast from player feet position
    static constexpr auto r = 0.1f;
    auto collision =
        CastMultipleRays({position + glm::vec3(0.0f, -radius + r, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)},
                         {glm::vec3(0.0f), glm::vec3(-r, 0, 0), glm::vec3(r, 0, 0), glm::vec3(0, 0, -r), glm::vec3(0, 0, r)}, world, 0.4f);

    if (!collision.has_value()) return false;
    const auto& value = collision.value();

    auto vel = velocity;
    vel.y = 0.0f;

    auto slopeRotation = glm::rotation(glm::vec3(0.0f, 1.0f, 0.0f), value.surfaceNormal);
    auto adjustedVelocity = slopeRotation * vel;

    // moving up the slope, no need to adjust velocity
    if (adjustedVelocity.y >= 0.0f) return false;

    adjustedVelocity = glm::normalize(adjustedVelocity) * length;

    velocity = adjustedVelocity;

    return true;
}

void Entity::Update(const TilesVec& world, float deltaTime, bool disableCollision) {
    static constexpr auto eps = 0.00000001f;

    if (glm::length(acceleration) <= eps) {
        ++standingStill;
    } else {
        standingStill = 0;
    }

    velocity += acceleration * GetFriction() * deltaTime;
    acceleration = glm::vec3();
    grounded = false;

    // adjust velocity
    if (standingStill <= 10) {
        grounded = adjustVelocityToSlope(position, radius, velocity, world);
    }

    // collision detection

    // ResolveCollisionWithWorldContinous(GetCollider(), *this, world, deltaTime, disableCollision);
    ResolveCollisionWithWorldDiscrete(GetSphereCollider(), *this, world, deltaTime, disableCollision);

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

bool Entity::IsGrounded() const {
    return grounded;
}
