#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>

#include "Tile.h"
#include "../Assets.h"
#include "../Renderer.h"
#include "../Utility/Vector3D.h"
#include "../Texture.h"

struct PositionColor {
    glm::vec3 position;
    glm::vec3 color;
    float scale;
};

class InstancedModel {
 public:
    InstancedModel(size_t cnt_, BufferId buf_);
    ~InstancedModel();

    InstancedModel(InstancedModel const&) = delete;
    InstancedModel& operator=(InstancedModel const&) = delete;

    size_t cnt = 0;
    BufferId buf = 0;
};

class TileRenderer {
 public:
    TileRenderer();

    void InitInstancedRendering(const std::vector<PositionColor>& tiles, const std::vector<PositionColor>& stairs);
    void RenderTilesInstanced();

 private:
    void InitRendering();

 private:
    Model cubeModel;
    InstancedModel cubeInstancedModel;
    Model stairsModel;
    InstancedModel stairsInstancedModel;

    Shader shader;
    Texture texture1 = Texture();
};
