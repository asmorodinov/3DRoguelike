#pragma once

#include "Tile.h"
#include "../Utility/Vector3D.h"

using TilesVec = Vector3D<Tile>;

bool IsAir(const Coordinates& coords, const TilesVec& world);