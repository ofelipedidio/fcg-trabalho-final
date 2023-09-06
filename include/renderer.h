#pragma once

#ifndef RENDERER
#define RENDERER

#include "glad/glad.h"
#include "GLFW/glfw3.h"

struct Renderer {
    GLint model;
    GLint view;
    GLint projection;
    GLint g_bbox_min_uniform;
    GLint g_bbox_max_uniform;

    Renderer(GLuint gpuProgram) {
        this->model = glGetUniformLocation(gpuProgram, "model");
        this->view = glGetUniformLocation(gpuProgram, "view");
        this->projection = glGetUniformLocation(gpuProgram, "projection");
        this->projection = glGetUniformLocation(gpuProgram, "bbox_min");
        this->projection = glGetUniformLocation(gpuProgram, "bbox_max");
    }
};

#endif // RENDERER
