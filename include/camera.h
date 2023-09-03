#pragma once

#include "glm/mat4x4.hpp"
#include "glm/vec4.hpp"
#include "matrices.h"

namespace game {
    struct Camera {
        bool usePerspectiveProjection = true;

        // Perspective parameters
        float theta = 0.0f;
        float phi = 0.0f;
        float distance = 50.0f;
        float field_of_view = 3.141592 / 3.0f;

        // General parameters
        float nearPlane = -0.1f;
        float farPlane = -100000.0f;
        float screenRatio = 1.0f;

        void computeMatrices(glm::mat4 &view, glm::mat4 &projection);
    };
}