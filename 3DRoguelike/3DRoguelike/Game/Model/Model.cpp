#include "Model.h"

#include "../Assets.h"

void Move(ModelData& faces, glm::vec3 offset) {
    for (auto& face : faces) {
        for (auto& vertex : face) {
            vertex.position += offset;
        }
    }
}

void RotateY(ModelData& faces) {
    for (auto& face : faces) {
        for (auto& vertex : face) {
            auto x = vertex.position.x;
            auto z = vertex.position.z;
            vertex.position.x = -z;
            vertex.position.z = x;
        }
    }
}

void RotateX(ModelData& faces) {
    for (auto& face : faces) {
        for (auto& vertex : face) {
            auto y = vertex.position.y;
            auto z = vertex.position.z;
            vertex.position.y = -z;
            vertex.position.z = y;
        }
    }
}

ModelData GetStairsModelData(int rotation) {
    auto data = Assets::GetModelData("stairs.obj");

    for (int i = 0; i < rotation % 4; ++i) {
        RotateY(data);
    }

    return data;
}

ModelData GetSlopeModelData(int rotation) {
    auto data = Assets::GetModelData("slope.obj");

    for (int i = 0; i < rotation % 4; ++i) {
        RotateY(data);
    }

    return data;
}
