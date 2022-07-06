#pragma once

#include <vector>

#include <glm/glm.hpp>

struct Edge {
    size_t v1;
    size_t v2;
};

std::vector<Edge> Delaunay3D(const std::vector<glm::vec3>& points);
