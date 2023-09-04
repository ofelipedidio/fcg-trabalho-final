#pragma once

#include <algorithm>
#include <vector>

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <queue>
#include <utility>

// Headers abaixo são específicos de C++
#include <iostream>
#include <map>
#include <ostream>
#include <stack>
#include <string>
#include <limits>
#include <fstream>
#include <sstream>

// Headers das bibliotecas OpenGL
#include "glad/glad.h"   // Criação de contexto OpenGL 3.3
#include "GLFW/glfw3.h"  // Criação de janelas do sistema operacional

#include "glm/mat4x4.hpp"
#include "glm/vec4.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "utils.h"
#include "matrices.h"
#include "renderer.h"
#include "object.h"

namespace Emitter {
    namespace _internal {
        template<typename F, typename S>
        struct CompareFirst {
            bool operator()(const std::pair<F, S>& a, const std::pair<F, S>& b) const {
                return a.first > b.first;
            }
        };
    }

    typedef struct ParticleProprieties {
        // Position
        float x, y, z;
        float xa, ya, za;
        // Rotation
        float rotationSpeedX, rotationSpeedY, rotationSpeedZ;
        // Duration
        float duration;
        // Size
        float initialSize, finalSize;
        // Object
        RenderObject object;
    } ParticleProprieties;

    class ParticleEmitter {
    public:
        ParticleProprieties proprieties;
        struct Particle {
            float x, y, z;
            float xs, ys, zs;
            float startSize;
            float life;
        };

        std::vector<Particle> particles;
        std::priority_queue<std::pair<float, Particle>, std::vector<std::pair<float, Particle>>, _internal::CompareFirst<float, Particle>> queue;
        float time = 0.0f;
        unsigned long int particleStart;
        unsigned long int particleEnd;

    public:
        ParticleEmitter(int maxParticleCount, ParticleProprieties proprieties);
        void emit(float x, float y, float z, float xs, float ys, float zs, float startSize);
        void emitIn(float x, float y, float z, float xs, float ys, float zs, float startSize, float timeToEmit);
        void onUpdate(float dt);
        void onRender(Renderer &renderer);
    };
}

