#include "TileRenderer.h"

#include "../Utility/GLError.h"

TileRenderer::TileRenderer()
    : cubeModel(GetStairsModel()), shader(Assets::GetShader("cubeShader.vs", "cubeShader.fs")), texture1(Assets::GetTexture("texture3.png")) {
}

TileRenderer::~TileRenderer() {
    // clean up instancing buffer
    if (cnt != 0) {
        glDeleteBuffers(1, &buf);
    }
}

void TileRenderer::InitInstancedRendering(const std::vector<PositionColor>& tiles) {
    // configure instanced array
    // -------------------------
    BufferId buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, tiles.size() * sizeof(PositionColor), &tiles[0], GL_STATIC_DRAW);

    glBindVertexArray(cubeModel.vao);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(PositionColor), (void*)0);
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(PositionColor), (void*)(sizeof(glm::vec3)));
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(PositionColor), (void*)(2 * sizeof(glm::vec3)));

    glVertexAttribDivisor(2, 1);
    glVertexAttribDivisor(3, 1);
    glVertexAttribDivisor(4, 1);

    glBindVertexArray(0);

    if (cnt != 0) {
        glDeleteBuffers(1, &buf);
    }
    cnt = tiles.size();
    buf = buffer;
}

void TileRenderer::RenderTilesInstanced() {
    InitRendering();
    glDrawArraysInstanced(GL_TRIANGLES, 0, cubeModel.triangleCount * 3, cnt);
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
