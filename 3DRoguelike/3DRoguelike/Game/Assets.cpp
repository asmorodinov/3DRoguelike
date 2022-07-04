#include "Assets.h"

Assets& Assets::Get() {
    static Assets me;
    return me;
}

Shader& Assets::GetShader(const std::string& vertexShader, const std::string& fragmentShader) {
    auto index = vertexShader + " | " + fragmentShader;
    auto& shaders = Assets::Get().shaders;

    if (!shaders.contains(index)) {
        shaders[index] = Shader(vertexShader, fragmentShader);
    }

    return shaders[index];
}
