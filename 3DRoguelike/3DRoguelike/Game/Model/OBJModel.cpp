#include "OBJModel.h"

#include <fstream>

#include "../Assert.h"

namespace OBJ {

ModelData LoadFromOBJ(const std::string& fileName) {
    auto prefix = std::string("../../../../3DRoguelike/");
    auto prefix2 = std::string("Resources/Models/");

    auto file = std::ifstream(prefix + prefix2 + fileName);
    LOG_ASSERT(file);

    auto mesh = ModelData();

    auto s = std::string();
    while (file >> s) {
        if (s == "v") {
            float x, y, z;
            file >> x >> y >> z;
            mesh.vertices.push_back(glm::vec3(x, y, z));
        } else if (s == "vt") {
            float u, v;
            file >> u >> v;
            mesh.textureCoords.push_back({u, v});
        } else if (s == "vn") {
            float x, y, z;
            file >> x >> y >> z;
            mesh.normals.push_back({x, y, z});
        } else if (s == "f") {
            size_t i, j, k;
            size_t ti, tj, tk;
            size_t ni, nj, nk;
            char c;
            file >> i >> c >> ti >> c >> ni >> j >> c >> tj >> c >> nj >> k >> c >> tk >> c >> nk;
            mesh.faces.push_back({i - 1, j - 1, k - 1, ti - 1, tj - 1, tk - 1, ni - 1, nj - 1, nk - 1});
        } else {
            std::string line;
            std::getline(file, line);
        }
    }

    file.close();

    return mesh;
}

}  // namespace OBJ
