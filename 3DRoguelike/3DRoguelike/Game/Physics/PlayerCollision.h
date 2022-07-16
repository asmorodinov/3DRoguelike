#pragma once

#include <vector>

#include "CollisionDetection.h"
#include "../Dungeon/WorldGrid.h"

// continous collision detection using swept AABB algorithm
void ResolveCollisionWithWorldContinous(const Box3D& boxCollider, MovingObject& object, const TilesVec& world, float deltaTime,
                                        bool disableCollision);

// discrete collision detection
void ResolveCollisionWithWorldDiscrete(const Sphere& sphereCollider, MovingObject& object, const TilesVec& world, float deltaTime,
                                       bool disableCollision);

// ray cast method
// needed to fix issue of player bouncing when moving down stairs:
// 1. first we cast a ray down from player position
// 2. then we project velocity of the player onto the surface of ray intersection surface
// this ensures that player is moving parallel to the surface of slope (stairs), and thus fixes issue
RayIntersectionResult RayCast(const Ray& ray, const TilesVec& world, Length maxLength);

RayIntersectionResult CastMultipleRays(const Ray& ray, const std::vector<glm::vec3>& originDeltas, const TilesVec& world, Length maxLength);
