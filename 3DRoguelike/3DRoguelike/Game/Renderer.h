#pragma once

using VAO = unsigned int;
using VBO = unsigned int;
using BufferId = unsigned int;

struct Model {
    VAO vao;
    VBO vbo;
    unsigned int triangleCount;
};

Model GetCubeModel();

void BindModel(const Model& model);
void RenderModel(const Model& model);

void DeleteModel(Model& model);
