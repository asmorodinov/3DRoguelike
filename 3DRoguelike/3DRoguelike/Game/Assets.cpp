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

const Texture& Assets::GetTexture(const std::string& name) {
    auto& textures = Assets::Get().textures;

    if (!textures.contains(name)) {
        textures[name] = LoadTexture(name);
    }

    return textures[name];
}

const ModelData& Assets::GetModelData(const std::string& name) {
    auto& models = Assets::Get().models;

    if (!models.contains(name)) {
        models[name] = OBJToModel(OBJ::LoadFromOBJ(name));
    }

    return models[name];
}
