#pragma once

#include <string>
#include <unordered_map>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "../Texture.h"
#include "../Shader.h"
#include "../Renderer.h"

/// Holds all state information relevant to a character as loaded using FreeType
struct Character {
    Texture TextureID;     // ID handle of the glyph texture
    glm::ivec2 Size;       // Size of glyph
    glm::ivec2 Bearing;    // Offset from baseline to left/top of glyph
    unsigned int Advance;  // Horizontal offset to advance to next glyph
};

class TextRenderer {
 public:
    TextRenderer();

    void LoadFont(const std::string& file);
    void RenderText(const std::string& text, glm::vec2 coords, float scale, glm::vec3 color);

 private:
    std::unordered_map<unsigned char, Character> characters;
    VAO vao;
    VBO vbo;
    Shader shader;
};