#include "ModelConverter.h"

ModelData OBJToModel(const OBJ::ModelData& data) {
    auto res = ModelData();

    for (const auto& face : data.faces) {
        const auto& p0 = data.vertices[face.i];
        const auto& p1 = data.vertices[face.j];
        const auto& p2 = data.vertices[face.k];
        const auto& uv0 = data.textureCoords[face.ti];
        const auto& uv1 = data.textureCoords[face.tj];
        const auto& uv2 = data.textureCoords[face.tk];

        res.push_back(Face{Vertex{p0, uv0}, Vertex{p1, uv1}, Vertex{p2, uv2}});
    }

    return res;
}
