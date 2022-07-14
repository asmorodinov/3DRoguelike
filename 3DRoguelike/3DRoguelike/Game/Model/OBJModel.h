#pragma once

#include <string>
#include <vector>

#include <glm/glm.hpp>

namespace OBJ {

struct Face {
    // vertex indices
    size_t i, j, k;
    // texture indices
    size_t ti, tj, tk;
    // normal indices
    size_t ni, nj, nk;
};

struct ModelData {
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> textureCoords;
    std::vector<Face> faces;
};

ModelData LoadFromOBJ(const std::string& fileName);

}  // namespace OBJ
