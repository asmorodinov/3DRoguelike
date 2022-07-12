#pragma once

#include "CollisionDetection.h"
#include "../Dungeon/WorldGrid.h"

// continous collision detection using swept AABB algorithm
void ResolveCollisionWithWorldContinous(const Box3D& boxCollider, MovingObject& object, const TilesVec& world, float deltaTime,
                                        bool disableCollision);

// discrete collision detection
void ResolveCollisionWithWorldDiscrete(const Sphere& sphereCollider, MovingObject& object, const TilesVec& world, float deltaTime,
                                       bool disableCollision);
