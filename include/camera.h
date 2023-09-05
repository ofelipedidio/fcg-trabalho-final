#pragma once

#include "glm/mat4x4.hpp"
#include "glm/vec4.hpp"
#include "matrices.h"
#include "movement.h"
#include "bezier.h"

namespace game
{
  struct Camera
  {
    // Perspective parameters
    bool usePerspectiveProjection = true;

    float distance = 50.0f;
    float field_of_view = 3.141592 / 3.0f;

    // General parameters
    float nearPlane = -0.1f;
    float farPlane = -100000.0f;
    float screenRatio = 1.0f;
    float width = 800.0f;
    float height = 800.0f;
    glm::vec4 position = glm::vec4(0, 0, 0, 1);
    glm::vec4 upVector = glm::vec4(0, 1, 0, 0);
    glm::vec4 viewVector = glm::vec4(0, 0, 1, 0);

    // Look at
    bool isLookAt = true;
    glm::vec4 lookAtPoint = glm::vec4(0.0f, 10.0f, 0.0f, 1.0f);
    float theta = 0.0f;
    float phi = 0.0f;

    // Bezier
    game::Movement movement;
    game::BezierCurve bezierCurve;
    float bezierTime = 2.0f;
    float bezierDuration = 5.0f;

    void computeMatrices(glm::mat4 &view, glm::mat4 &projection);
    void onUpdate(float deltaTime);
  };
}