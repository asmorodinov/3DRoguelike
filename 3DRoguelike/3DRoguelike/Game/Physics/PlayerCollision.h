#pragma once

#include "CollisionDetection.h"
#include "../Dungeon/WorldGrid.h"

void ResolveCollisionWithWorld(const Box3D& boxCollider, MovingObject& object, const TilesVec& world, float deltaTime, bool disableCollision);