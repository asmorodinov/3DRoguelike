#pragma once

#include <string>
#include <unordered_map>

#include "Shader.h"

class Assets {
 public:
    static Assets& Get();

    Assets(Assets const&) = delete;
    void operator=(Assets const&) = delete;

    static Shader& GetShader(const std::string& vertexShader, const std::string& fragmentShader);

 private:
    Assets() = default;

 private:
    std::unordered_map<std::string, Shader> shaders;
};