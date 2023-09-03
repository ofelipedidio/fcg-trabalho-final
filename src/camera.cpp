#include "camera.h"

#include "glm/gtc/type_ptr.hpp"
#include "glad/glad.h"
#include "GLFW/glfw3.h"

namespace game {
    void Camera::computeMatrices(glm::mat4 &view, glm::mat4 &projection) {
        // View
        {
            float r = distance;
            float y = r * sin(phi);
            float z = r * cos(phi) * cos(theta);
            float x = r * cos(phi) * sin(theta);
            glm::vec4 camera_position_c = glm::vec4(x, y, z, 1.0f);
            glm::vec4 camera_lookat_l = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
            glm::vec4 camera_view_vector = camera_lookat_l - camera_position_c;
            glm::vec4 camera_up_vector = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
            view = Matrix_Camera_View(camera_position_c, camera_view_vector, camera_up_vector);
        }

        // Projection
        {
            if (usePerspectiveProjection) {
                projection = Matrix_Perspective(field_of_view, screenRatio, nearPlane, farPlane);
            } else {
                float t = 1.5f * distance / 2.5f;
                float b = -t;
                float r = t * screenRatio;
                float l = -r;
                projection = Matrix_Orthographic(l, r, b, t, nearPlane, farPlane);
            }
        }
    }
}