#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <unordered_map>

#include "Texture.h"
#include "Shader.h"

#include "Model/Model.h"
#include "Model/ModelConverter.h"

class Assets {
 public:
    static Assets& Get();

    Assets(Assets const&) = delete;
    void operator=(Assets const&) = delete;

    static Shader& GetShader(const std::string& vertexShader, const std::string& fragmentShader);
    static const Texture& GetTexture(const std::string& name);
    static const ModelData& GetModelData(const std::string& name);

 public:
    glm::mat4 orthogonalProjection;
    glm::mat4 projection;
    glm::mat4 view;

 private:
    Assets() = default;

 private:
    std::unordered_map<std::string, Shader> shaders;
    std::unordered_map<std::string, Texture> textures;
    std::unordered_map<std::string, ModelData> models;
};
