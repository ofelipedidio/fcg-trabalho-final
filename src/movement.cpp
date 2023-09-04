#include "movement.h"
#include <iostream>

namespace game {
    void Movement::update(float deltaTime) {
        float catch_up = 0.99f;

        float _xs = (incX ? 1.0f : 0.0f) - (decX ? 1.0f : 0.0f);
        float _ys = (incY ? 1.0f : 0.0f) - (decY ? 1.0f : 0.0f);
        float _zs = (incZ ? 1.0f : 0.0f) - (decZ ? 1.0f : 0.0f);
        xs = _xs;
        ys = _ys;
        zs = _zs;

        return;
        deltaTime += loss;

        float i = 0.0f;
        while (i < deltaTime) {
            i += 0.1f;

            xs *= 1.0f - catch_up;
            xs += catch_up * _xs;
            ys *= 1.0f - catch_up;
            ys += catch_up * _ys;
            zs *= 1.0f - catch_up;
            zs += catch_up * _zs;
        }

        loss = deltaTime - i;
        std::cout << loss << std::endl;
    }
}