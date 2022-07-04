#pragma once

using VAO = unsigned int;
using VBO = unsigned int;

struct Model {
    VAO vao;
    VBO vbo;
    size_t triangleCount;
};

Model GetCubeModel();

void RenderModel(const Model& model);

void DeleteModel(Model& model);
