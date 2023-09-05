#include "bezier.h"

namespace game
{
  glm::vec4 lerp(glm::vec4 &start, glm::vec4 &end, float t)
  {
    return (start * (1.0f - t)) + (end * t);
  }

  glm::vec4 BezierCurve::at(float t)
  {
    glm::vec4 p01 = lerp(p0, p1, t);
    glm::vec4 p12 = lerp(p1, p2, t);
    glm::vec4 p23 = lerp(p2, p3, t);

    glm::vec4 p012 = lerp(p01, p12, t);
    glm::vec4 p123 = lerp(p12, p23, t);

    return lerp(p012, p123, t);
  }
}