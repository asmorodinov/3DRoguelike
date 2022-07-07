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

class TileRenderer {
 public:
    TileRenderer();
    ~TileRenderer();

    void InitInstancedRendering(const std::vector<PositionColor>& tiles);
    void RenderTilesInstanced();

 private:
    void InitRendering();

 private:
    Model cubeModel;
    Shader shader;
    Texture texture1 = Texture();

    size_t cnt = 0;
    BufferId buf = 0;
};
