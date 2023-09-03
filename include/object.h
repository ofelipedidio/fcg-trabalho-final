#pragma once

#include <GLFW/glfw3.h>

struct RenderObject {
    void* vertexes;
    int vertexCount;
    GLenum renderingMode;

    RenderObject() {}
    RenderObject (void* vertexes, int vertexCount, GLenum renderingMode) {
        this->vertexes = vertexes;
        this->vertexCount = vertexCount;
        this->renderingMode = renderingMode;
    }

    inline void draw() {
        glDrawElements(this->renderingMode, this->vertexCount, GL_UNSIGNED_INT, this->vertexes);
    }
};
