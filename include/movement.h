#pragma once

namespace game {
    struct Movement {
        float xs, ys, zs;
        float loss = 0.0f;

        bool incX = false;
        bool decX = false;
        bool incY = false;
        bool decY = false;
        bool incZ = false;
        bool decZ = false;

        void update(float deltaTime);
    };
}
