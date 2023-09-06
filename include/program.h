#pragma once

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "camera.h"

namespace game
{
  struct Game
  {
    game::Camera *camera;

    void onUpdate(float timeDelta);
    void onRender();
    void gameLoop();
  };
}
