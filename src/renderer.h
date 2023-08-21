#pragma once

#ifndef RENDERER
#define RENDERER

#include "glad/glad.h"
#include "GLFW/glfw3.h"

struct Renderer {
    GLint model;
    GLint view;
    GLint projection;

    Renderer(GLuint gpuProgram) {
        this->model = glGetUniformLocation(gpuProgram, "model");
        this->view = glGetUniformLocation(gpuProgram, "view");
        this->projection = glGetUniformLocation(gpuProgram, "projection");
    }
};

#endif // RENDERER
