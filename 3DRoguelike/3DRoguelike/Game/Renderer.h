#pragma once

using VAO = unsigned int;
using VBO = unsigned int;
using BufferId = unsigned int;

class Model {
 public:
    Model(VAO vao_, VBO vbo_, unsigned int tc);
    ~Model();

    Model(Model const&) = delete;
    Model& operator=(Model const&) = delete;

    VAO vao = 0;
    VBO vbo = 0;
    unsigned int triangleCount = 0;
};

Model GetCubeModel();
Model GetStairsModel();

void BindModel(const Model& model);
void RenderModel(const Model& model);
