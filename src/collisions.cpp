#include "collisions.h"
#include <cmath>
#include <limits>

#define FLOAT_PRECISION 1e-5

namespace collision
{
  Point Ray::at(float t)
  {
    return {startPosition.x + t * direction.x,
            startPosition.y + t * direction.y,
            startPosition.z + t * direction.z};
  }

  bool check(Cube &cube1, Cube &cube2)
  {
    return (cube1.positionMin.x <= cube2.positionMax.x && cube1.positionMax.x >= cube2.positionMin.x) &&
           (cube1.positionMin.y <= cube2.positionMax.y && cube1.positionMax.y >= cube2.positionMin.y) &&
           (cube1.positionMin.z <= cube2.positionMax.z && cube1.positionMax.z >= cube2.positionMin.z);
  }

  // TODO: check if code is 100$ sound
  bool check(Cube &cube, Plane &plane)
  {
    // Calculate distance from the cube center to the plane
    Point center;
    center.x = (cube.positionMin.x + cube.positionMax.x) / 2;
    center.y = (cube.positionMin.y + cube.positionMax.y) / 2;
    center.z = (cube.positionMin.z + cube.positionMax.z) / 2;

    float distance = plane.normal.x * (center.x - plane.position.x) +
                     plane.normal.y * (center.y - plane.position.y) +
                     plane.normal.z * (center.z - plane.position.z);

    if (distance == 0)
    {
      return true; // The cube's center lies on the plane
    }

    // Compute the half-diagonal length of the cube
    float halfDiagonalLength = sqrt(pow(cube.positionMax.x - center.x, 2) +
                                    pow(cube.positionMax.y - center.y, 2) +
                                    pow(cube.positionMax.z - center.z, 2));

    return fabs(distance) <= halfDiagonalLength;
  }

  // TODO: check if code is 100$ sound
  bool check(Sphere &sphere, Point &point)
  {
    float distance = sqrt(pow(sphere.position.x - point.x, 2) +
                          pow(sphere.position.y - point.y, 2) +
                          pow(sphere.position.z - point.z, 2));

    return distance <= sphere.r;
  }

  float collide(Plane &plane, Ray &ray)
  {
    float dotProduct = plane.normal.x * ray.direction.x +
                       plane.normal.y * ray.direction.y +
                       plane.normal.z * ray.direction.z;

    // Check if the ray is parallel to the plane
    if (fabs(dotProduct) < FLOAT_PRECISION)
    {
      return -std::numeric_limits<float>::infinity();
    }

    float d = plane.normal.x * plane.position.x +
              plane.normal.y * plane.position.y +
              plane.normal.z * plane.position.z;

    float t = (d - (plane.normal.x * ray.startPosition.x +
                    plane.normal.y * ray.startPosition.y +
                    plane.normal.z * ray.startPosition.z)) /
              dotProduct;

    return t;
  }
}