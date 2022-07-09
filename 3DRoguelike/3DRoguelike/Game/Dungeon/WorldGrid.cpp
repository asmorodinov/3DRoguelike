#include "WorldGrid.h"

bool IsAir(const Coordinates& coords, const TilesVec& world) {
    if (!coords.IsInBounds(world.GetDimensions())) {
        return false;
    }
    return IsAir(world.Get(coords).type);
}
