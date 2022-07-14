#include "Renderer.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include "Assets.h"

GLModel SendModelDataToGPU(const ModelData& data) {
    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    VBO vbo;
    VAO vao;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Face) * data.size(), &data[0], GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    return {vao, vbo, static_cast<unsigned int>(data.size())};
}

GLModel GetCubeModel() {
    const auto& data = Assets::GetModelData("cube.obj");
    return SendModelDataToGPU(data);
}

GLModel GetStairsModel(int rotation) {
    auto data = GetStairsModelData(rotation);
    return SendModelDataToGPU(data);
}

void BindModel(const GLModel& model) {
    glBindVertexArray(model.vao);
}

void RenderModel(const GLModel& model) {
    glDrawArrays(GL_TRIANGLES, 0, model.triangleCount * 3);
}

GLModel::GLModel(VAO vao_, VBO vbo_, unsigned int tc) : vao(vao_), vbo(vbo_), triangleCount(tc) {
}

GLModel::~GLModel() {
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
}
