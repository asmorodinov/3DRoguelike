#include "Model.h"

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
    auto data = ModelData{
        {Vertex{{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}}, Vertex{{0.5f, 0.5f, -0.5f}, {1.0f, 1.0f}}, Vertex{{0.5f, -0.5f, -0.5f}, {1.0f, 0.0f}}},
        {Vertex{{0.5f, 0.5f, -0.5f}, {1.0f, 1.0f}}, Vertex{{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}}, Vertex{{-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f}}},
        {Vertex{{-0.5f, -0.5f, 1.5f}, {1.0f, 0.0f}}, Vertex{{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f}}, Vertex{{-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f}}},
        {Vertex{{0.5f, -0.5f, 1.5f}, {1.0f, 0.0f}}, Vertex{{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f}}, Vertex{{0.5f, 0.5f, -0.5f}, {1.0f, 1.0f}}},
        {Vertex{{-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f}}, Vertex{{0.5f, -0.5f, -0.5f}, {1.0f, 1.0f}}, Vertex{{0.5f, -0.5f, 1.5f}, {1.0f, 0.0f}}},
        {Vertex{{0.5f, -0.5f, 1.5f}, {1.0f, 0.0f}}, Vertex{{-0.5f, -0.5f, 1.5f}, {0.0f, 0.0f}}, Vertex{{-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f}}},
        {Vertex{{-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f}}, Vertex{{0.5f, -0.5f, 1.5f}, {1.0f, 0.0f}}, Vertex{{0.5f, 0.5f, -0.5f}, {1.0f, 1.0f}}},
        {Vertex{{0.5f, -0.5f, 1.5f}, {1.0f, 0.0f}}, Vertex{{-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f}}, Vertex{{-0.5f, -0.5f, 1.5f}, {0.0f, 0.0f}}}};

    auto data2 = data;
    RotateX(data2);
    RotateX(data2);
    Move(data2, glm::vec3(0, 1, 1));
    data.insert(data.end(), data2.begin(), data2.end());

    for (int i = 0; i < rotation % 4; ++i) {
        RotateY(data);
    }

    return data;
}
