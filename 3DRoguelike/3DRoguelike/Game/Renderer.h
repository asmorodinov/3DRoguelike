#pragma once

#include "Model/Model.h"

using VAO = unsigned int;
using VBO = unsigned int;
using BufferId = unsigned int;

class GLModel {
 public:
    GLModel(VAO vao_, VBO vbo_, unsigned int tc);
    ~GLModel();

    GLModel(GLModel const&) = delete;
    GLModel& operator=(GLModel const&) = delete;

    VAO vao = 0;
    VBO vbo = 0;
    unsigned int triangleCount = 0;
};

GLModel SendModelDataToGPU(const ModelData& data);

GLModel GetCubeModel();
GLModel GetStairsModel(int rotation);

void BindModel(const GLModel& model);
void RenderModel(const GLModel& model);
