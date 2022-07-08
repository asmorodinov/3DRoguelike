#include "Renderer.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include <vector>

Model GetCubeModel() {
    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    static const float vertices[] = {-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.5f,  0.5f,  -0.5f, 1.0f, 1.0f, 0.5f,  -0.5f, -0.5f, 1.0f, 0.0f,
                                     0.5f,  0.5f,  -0.5f, 1.0f, 1.0f, -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -0.5f, 0.5f,  -0.5f, 0.0f, 1.0f,

                                     -0.5f, -0.5f, 0.5f,  0.0f, 0.0f, 0.5f,  -0.5f, 0.5f,  1.0f, 0.0f, 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
                                     0.5f,  0.5f,  0.5f,  1.0f, 1.0f, -0.5f, 0.5f,  0.5f,  0.0f, 1.0f, -0.5f, -0.5f, 0.5f,  0.0f, 0.0f,

                                     -0.5f, 0.5f,  0.5f,  1.0f, 0.0f, -0.5f, 0.5f,  -0.5f, 1.0f, 1.0f, -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
                                     -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, -0.5f, -0.5f, 0.5f,  0.0f, 0.0f, -0.5f, 0.5f,  0.5f,  1.0f, 0.0f,

                                     0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.5f,  -0.5f, -0.5f, 0.0f, 1.0f, 0.5f,  0.5f,  -0.5f, 1.0f, 1.0f,
                                     0.5f,  -0.5f, -0.5f, 0.0f, 1.0f, 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.5f,  -0.5f, 0.5f,  0.0f, 0.0f,

                                     -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.5f,  -0.5f, -0.5f, 1.0f, 1.0f, 0.5f,  -0.5f, 0.5f,  1.0f, 0.0f,
                                     0.5f,  -0.5f, 0.5f,  1.0f, 0.0f, -0.5f, -0.5f, 0.5f,  0.0f, 0.0f, -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,

                                     -0.5f, 0.5f,  -0.5f, 0.0f, 1.0f, 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.5f,  0.5f,  -0.5f, 1.0f, 1.0f,
                                     0.5f,  0.5f,  0.5f,  1.0f, 0.0f, -0.5f, 0.5f,  -0.5f, 0.0f, 1.0f, -0.5f, 0.5f,  0.5f,  0.0f, 0.0f};

    VBO vbo;
    VAO vao;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    return {vao, vbo, 12};
}

struct Vertex {
    glm::vec3 position;
    glm::vec2 texCoords;
};

void move(std::vector<Vertex>& vertices, glm::vec3 offset) {
    for (auto& vertex : vertices) {
        vertex.position += offset;
    }
}

void rotateY(std::vector<Vertex>& vertices) {
    for (auto& vertex : vertices) {
        auto x = vertex.position.x;
        auto z = vertex.position.z;
        vertex.position.x = -z;
        vertex.position.z = x;
    }
}
void rotateX(std::vector<Vertex>& vertices) {
    for (auto& vertex : vertices) {
        auto y = vertex.position.y;
        auto z = vertex.position.z;
        vertex.position.y = -z;
        vertex.position.z = y;
    }
}

Model GetStairsModel(int rotation) {
    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------

    auto vertices =
        std::vector<Vertex>{{{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}}, {{0.5f, 0.5f, -0.5f}, {1.0f, 1.0f}},   {{0.5f, -0.5f, -0.5f}, {1.0f, 0.0f}},
                            {{0.5f, 0.5f, -0.5f}, {1.0f, 1.0f}},   {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}}, {{-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f}},
                            {{-0.5f, -0.5f, 1.5f}, {1.0f, 0.0f}},  {{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f}},  {{-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f}},
                            {{0.5f, -0.5f, 1.5f}, {1.0f, 0.0f}},   {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f}},  {{0.5f, 0.5f, -0.5f}, {1.0f, 1.0f}},
                            {{-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f}}, {{0.5f, -0.5f, -0.5f}, {1.0f, 1.0f}},  {{0.5f, -0.5f, 1.5f}, {1.0f, 0.0f}},
                            {{0.5f, -0.5f, 1.5f}, {1.0f, 0.0f}},   {{-0.5f, -0.5f, 1.5f}, {0.0f, 0.0f}},  {{-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f}},
                            {{-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f}},  {{0.5f, -0.5f, 1.5f}, {1.0f, 0.0f}},   {{0.5f, 0.5f, -0.5f}, {1.0f, 1.0f}},
                            {{0.5f, -0.5f, 1.5f}, {1.0f, 0.0f}},   {{-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f}},  {{-0.5f, -0.5f, 1.5f}, {0.0f, 0.0f}}};

    auto v2 = vertices;
    rotateX(v2);
    rotateX(v2);
    move(v2, glm::vec3(0, 1, 1));
    vertices.insert(vertices.end(), v2.begin(), v2.end());

    for (int i = 0; i < rotation; ++i) {
        rotateY(vertices);
    }

    VBO vbo;
    VAO vao;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    return {vao, vbo, static_cast<unsigned int>(vertices.size() / 3)};
}

void BindModel(const Model& model) {
    glBindVertexArray(model.vao);
}

void RenderModel(const Model& model) {
    glDrawArrays(GL_TRIANGLES, 0, model.triangleCount * 3);
}

Model::Model(VAO vao_, VBO vbo_, unsigned int tc) : vao(vao_), vbo(vbo_), triangleCount(tc) {
}

Model::~Model() {
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
}
