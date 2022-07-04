#pragma once

#include <string>
#include <unordered_map>

#include "Texture.h"
#include "Shader.h"

class Assets {
 public:
    static Assets& Get();

    Assets(Assets const&) = delete;
    void operator=(Assets const&) = delete;

    static Shader& GetShader(const std::string& vertexShader, const std::string& fragmentShader);
    static const Texture& GetTexture(const std::string& name);

 private:
    Assets() = default;

 private:
    std::unordered_map<std::string, Shader> shaders;
    std::unordered_map<std::string, Texture> textures;
};
