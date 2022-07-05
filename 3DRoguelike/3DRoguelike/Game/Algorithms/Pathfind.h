#pragma once

#include <vector>

#include "../Utility/Random.h"
#include "../Dungeon/WorldGrid.h"

std::vector<Coordinates> RandomPath(const std::vector<Coordinates>& start, const std::vector<Coordinates>& finish, const TilesVec& world, RNG& rng);
void PlacePath(const std::vector<Coordinates>& path, TilesVec& world, const Tile& wall, const Tile& air);
