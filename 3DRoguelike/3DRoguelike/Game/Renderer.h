#pragma once

using VAO = unsigned int;
using VBO = unsigned int;
using BufferId = unsigned int;

struct Model {
    VAO vao = 0;
    VBO vbo = 0;
    unsigned int triangleCount = 0;
};

Model GetCubeModel();

void BindModel(const Model& model);
void RenderModel(const Model& model);

void DeleteModel(Model& model);
