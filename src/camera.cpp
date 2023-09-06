#include "camera.h"

#include "glm/gtc/type_ptr.hpp"
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include <iostream>

namespace game
{
  void Camera::computeMatrices(glm::mat4 &view, glm::mat4 &projection)
  {
    // View
    {
      if (isLookAt)
      {
        float r = distance;
        if (!(0.0f <= bezierTime && bezierTime <= 1.0f))
        {
            float y = (r * std::sin(phi));
            float z = (r * std::cos(phi) * std::cos(theta));
            float x = (r * std::cos(phi) * std::sin(theta));
            position = glm::vec4(x, y, z, 1.0f);
            viewVector = -position;
            viewVector.w = 0.0f;
            position.x += lookAtPoint.x;
            position.y += lookAtPoint.y;
            position.z += lookAtPoint.z;
        } else {
            viewVector = lookAtPoint - position;
        }
        //viewVector /= norm(viewVector);
      }
      else
      {
        float y = std::sin(phi);
        float z = std::cos(phi) * std::cos(theta);
        float x = std::cos(phi) * std::sin(theta);
        viewVector = -glm::vec4(x, y, z, 0.0f);
      }
      view = Matrix_Camera_View(position, viewVector, upVector);
    }

    // Projection
    {
      if (usePerspectiveProjection)
      {
        projection = Matrix_Perspective(field_of_view, screenRatio, nearPlane, farPlane);
      }
      else
      {
        float t = 1.5f * distance / 2.5f;
        float b = -t;
        float r = t * screenRatio;
        float l = -r;
        projection = Matrix_Orthographic(l, r, b, t, nearPlane, farPlane);
      }
    }
  }

  void Camera::onUpdate(float deltaTime)
  {
    bezierTime += deltaTime / bezierDuration;
    if (0.0f <= bezierTime && bezierTime <= 1.0f)
    {
      if (bezierTime >= 1.0f)
      {
        position = bezierCurve.at(1.0f);
        bezierTime = 2.0f;
      }
      else
      {
        position = bezierCurve.at(bezierTime);
      }
    }

    if (isLookAt)
      return;

    movement.update(deltaTime);

#define print_vec4(v) std::cout << "(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")" << std::endl
    glm::vec4 rightVector = crossproduct(viewVector, upVector) * glm::vec4(1, 0, 1, 0);
    rightVector /= norm(rightVector);

    glm::vec4 forwardVector = viewVector * glm::vec4(1, 0, 1, 0);
    forwardVector /= norm(forwardVector);

    float speed = 25.0f;

    glm::vec4 mov = (movement.xs * rightVector) + (movement.ys * forwardVector) + (movement.zs * upVector);
    if (std::abs(mov.x) < 1e-3 && std::abs(mov.y) < 1e-3 && std::abs(mov.z) < 1e-3)
      return;
    mov *= deltaTime * speed / norm(mov);

    position += mov;
  }
}