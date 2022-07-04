#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Tile.h"
#include "../Assets.h"
#include "../Renderer.h"
#include "../Utility/Vector3D.h"
#include "../Texture.h"

class TileRenderer {
 public:
    TileRenderer();
    ~TileRenderer();

    void InitRendering();

    void RenderTile(const Coordinates& coordinates, const Tile& tile);
    void RenderTile(size_t x, size_t y, size_t z, const Tile& tile);

 private:
    Model cubeModel;
    Shader& shader;
    Texture texture1;
};