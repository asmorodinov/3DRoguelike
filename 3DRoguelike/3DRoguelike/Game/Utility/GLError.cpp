#include "GLError.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#define NOMINMAX
#include <spdlog/spdlog.h>

using namespace std;

void _check_gl_error(const char* file, int line) {
    GLenum err(glGetError());

    while (err != GL_NO_ERROR) {
        string error;

        switch (err) {
            case GL_INVALID_OPERATION:
                error = "INVALID_OPERATION";
                break;
            case GL_INVALID_ENUM:
                error = "INVALID_ENUM";
                break;
            case GL_INVALID_VALUE:
                error = "INVALID_VALUE";
                break;
            case GL_OUT_OF_MEMORY:
                error = "OUT_OF_MEMORY";
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                error = "INVALID_FRAMEBUFFER_OPERATION";
                break;
        }

        cerr << "GL_" << error.c_str() << " - " << file << ":" << line << endl;
        err = glGetError();
    }
}

void CheckOpenGLError(const char* stmt, const char* fname, int line) {
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        spdlog::error("OpenGL error {}, at {}:{} - for {}\n", err, fname, line, stmt);
        spdlog::default_logger()->flush();
        abort();
    }
}

void CheckAssert(const char* stmt, const char* fname, int line) {
    spdlog::error("Assertion error at {}:{} - for {}\n", fname, line, stmt);
    spdlog::default_logger()->flush();
    abort();
}
