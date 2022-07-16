#pragma once

#include <glm/glm.hpp>

#include "../Dungeon/WorldGrid.h"
#include "CollisionDetection.h"

static constexpr auto FLYING_ACCELERATION = glm::vec3(0);
static constexpr auto GRAVITY_ACCELERATION = glm::vec3(0, -32, 0);
static constexpr auto FRICTION = glm::vec3(20);
static constexpr auto DRAG_FLY = glm::vec3(5);
static constexpr auto DRAG_JUMP = glm::vec3(1.8f, 0.0f, 1.8f);
static constexpr auto DRAG_FALL = glm::vec3(1.8f, 0.4f, 1.8f);

class Entity : private MovingObject {
 public:
    Entity() = default;

    Sphere GetSphereCollider() const;
    Box3D GetCollider() const;

    void Jump(float jumpHeight_ = 0.0f);

    glm::vec3 GetFriction() const;

    void Update(const TilesVec& world, float deltaTime, bool disableCollision);

    // member variables setters and getters
    void SetPosition(const glm::vec3& pos);
    glm::vec3 GetPosition() const;
    void SetVelocity(const glm::vec3& vel);
    glm::vec3 GetVelocity() const;
    void SetAcceleration(const glm::vec3& acc);
    glm::vec3 GetAcceleration() const;
    glm::vec3& GetAcceleration();

    void SetFlying(bool flying_);
    bool IsFlying() const;

    bool IsGrounded() const;
    bool IsJumping() const;

 private:
    float jumpHeight = 1.25f;
    bool flying = false;
    bool jumping = false;

    glm::vec3 acceleration = glm::vec3();

    float width = 0.4f;
    float height = 0.7f;
    float radius = 0.3f;
};
