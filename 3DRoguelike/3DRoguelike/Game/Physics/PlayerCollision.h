#pragma once

#include "../Camera.h"
#include "../Dungeon/WorldGrid.h"

void ResolveCollisionWithWorld(Camera& camera, const TilesVec& world, float dt, bool disableCollision);
