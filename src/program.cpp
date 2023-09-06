#include "program.h"

namespace game
{
  void Game::gameLoop()
  {
    float previousTime = glfwGetTime();

    while (true)
    {
      // Compute deltaTime
      float now = glfwGetTime();
      float timeDelta = now - previousTime;
      previousTime = now;

      // Updates the game elements
      onUpdate(timeDelta);

      // Render the game elements
      onRender();
    }
  }

  void Game::onUpdate(float timeDelta)
  {
  }

  void Game::onRender()
  {
  }
}