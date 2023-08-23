#pragma once

#ifndef PARTICLE
#define PARTICLE

#include <algorithm>
#include <vector>

#include <cmath>
#include <cstdio>
#include <cstdlib>

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

namespace Particle {
    typedef struct ParticleProprieties {
        glm::vec4 p[4];
        // Object
        RenderObject object;
    } ParticleProprieties;

    class ParticleEmitter {
        private:
            struct Particle {
                ParticleProprieties props;
                float life;
            };

            std::vector<Particle> particles;
            unsigned long int particleStart;
            unsigned long int particleEnd;

        public:
            ParticleEmitter(int maxParticleCount);
            void emit(ParticleProprieties proprieties);
            void emitIn(ParticleProprieties proprieties, float timeToEmit);
            void onUpdate(float dt);
            void onRender(Renderer &renderer);
    };
}

#endif

