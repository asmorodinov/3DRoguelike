#include "TileRenderer.h"

TileRenderer::TileRenderer()
    : cubeModel(GetCubeModel()), shader(Assets::GetShader("cubeShader.vs", "cubeShader.fs")), texture1(Assets::GetTexture("texture.png")) {
}

TileRenderer::~TileRenderer() {
    DeleteModel(cubeModel);
}

void TileRenderer::InitRendering() {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture1);

    shader.use();
    shader.setInt("texture1", 0);

    shader.setMat4("projection", Assets::Get().projection);
    shader.setMat4("view", Assets::Get().view);

    BindModel(cubeModel);
}

void TileRenderer::RenderTile(const Coordinates& coordinates, const Tile& tile) {
    if (tile.type == TileType::Air || tile.type == TileType::Void) {
        return;
    }
    if (tile.texture == TextureType::None) {
        return;
    }

    shader.setVec3("offset", glm::vec3(coordinates.x, coordinates.y, coordinates.z));
    RenderModel(cubeModel);
}

void TileRenderer::RenderTile(size_t x, size_t y, size_t z, const Tile& tile) {
    RenderTile(Coordinates{x, y, z}, tile);
}
