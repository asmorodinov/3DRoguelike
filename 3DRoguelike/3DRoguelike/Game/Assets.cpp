#include "Assets.h"

#include "Utility/PathToResources.h"

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

const YAML::Node& Assets::GetYAMLFile(const std::string& name) {
    auto& yamlFiles = Assets::Get().yamlFiles;

    if (!yamlFiles.contains(name)) {
        yamlFiles[name] = YAML::LoadFile(pathToResources + "Configuration/"s + name + ".yaml");
    }

    return yamlFiles[name];
}

const YAML::Node& Assets::GetConfig() {
    return Assets::GetYAMLFile("config");
}

const bool Assets::HasConfigParameter(const std::string& name) {
    return static_cast<bool>(Assets::GetConfig()[name]);
}
