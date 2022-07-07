#include "TileRenderer.h"

#include "../Utility/GLError.h"

TileRenderer::TileRenderer()
    : cubeModel(GetCubeModel()),
      cubeInstancedModel(0, 0),
      stairsModel(GetStairsModel()),
      stairsInstancedModel(0, 0),
      shader(Assets::GetShader("cubeShader.vs", "cubeShader.fs")),
      texture1(Assets::GetTexture("texture3.png")) {
}

void initInstancedRendering(const Model& model, InstancedModel& instancedModel, const std::vector<PositionColor>& tiles) {
    if (tiles.empty()) {
        return;
    }

    // configure instanced array
    // -------------------------
    BufferId buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, tiles.size() * sizeof(PositionColor), &tiles[0], GL_STATIC_DRAW);

    glBindVertexArray(model.vao);
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

    if (instancedModel.cnt != 0) {
        glDeleteBuffers(1, &instancedModel.buf);
    }
    instancedModel.cnt = tiles.size();
    instancedModel.buf = buffer;
}

void TileRenderer::InitInstancedRendering(const std::vector<PositionColor>& tiles, const std::vector<PositionColor>& stairs) {
    initInstancedRendering(cubeModel, cubeInstancedModel, tiles);
    initInstancedRendering(stairsModel, stairsInstancedModel, stairs);
}

void TileRenderer::RenderTilesInstanced() {
    InitRendering();

    if (cubeInstancedModel.cnt != 0) {
        BindModel(cubeModel);
        glDrawArraysInstanced(GL_TRIANGLES, 0, cubeModel.triangleCount * 3, cubeInstancedModel.cnt);
    }
    if (stairsInstancedModel.cnt != 0) {
        BindModel(stairsModel);
        glDrawArraysInstanced(GL_TRIANGLES, 0, stairsModel.triangleCount * 3, stairsInstancedModel.cnt);
    }
}

void TileRenderer::InitRendering() {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture1);

    shader.use();
    shader.setInt("texture1", 0);

    shader.setMat4("projection", Assets::Get().projection);
    shader.setMat4("view", Assets::Get().view);
}

InstancedModel::InstancedModel(size_t cnt_, BufferId buf_) : cnt(cnt_), buf(buf_) {
}

InstancedModel::~InstancedModel() {
    if (cnt != 0) {
        glDeleteBuffers(1, &buf);
    }
}
