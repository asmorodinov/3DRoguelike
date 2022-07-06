#pragma once

#include <vector>

#include "Delaunay3D.h"

using Weight = float;

std::vector<Edge> MinimumSpanningTree(const std::vector<Edge>& graph, size_t vertices, const std::vector<Weight>& weights);
