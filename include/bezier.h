#pragma once

#include "glm/vec4.hpp"

namespace game {
    struct BezierCurve {
        glm::vec4 p0;
        glm::vec4 p1;
        glm::vec4 p2;
        glm::vec4 p3;

        glm::vec4 at(float t);
    };
}