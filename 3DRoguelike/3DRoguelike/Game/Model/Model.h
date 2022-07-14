#pragma once

#include <array>
#include <vector>

#include <glm/glm.hpp>

struct Vertex {
    glm::vec3 position;
    glm::vec2 texCoords;
};

using Face = std::array<Vertex, 3>;

using ModelData = std::vector<Face>;

void Move(ModelData& faces, glm::vec3 offset);
void RotateY(ModelData& faces);
void RotateX(ModelData& faces);

ModelData GetStairsModelData(int rotation);
ModelData GetSlopeModelData(int rotation);
