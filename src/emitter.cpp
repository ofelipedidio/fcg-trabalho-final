#include "emitter.h"
#include "matrices.h"
#include <iostream>
#include <stdint.h>

namespace Emitter {
    namespace _internal {
        uint32_t xorshift32(uint32_t state) {
            state ^= state << 13;
            state ^= state >> 17;
            state ^= state << 5;
            return state;
        }

        float generate_floats(uint32_t &state) {
            state = xorshift32(state);
            return ((float) (state % 1000000)) / 1000000.0f;  // Convert to a float in range [0, 1)
        }
    }

    ParticleEmitter::ParticleEmitter(int maxParticleCount, ParticleProprieties proprieties) {
        this->proprieties = proprieties;
        this->particles = std::vector<Particle>(maxParticleCount);
        this->particleStart = 0;
        this->particleEnd = 0;
    }

    void ParticleEmitter::emit(float x, float y, float z, float xs, float ys, float zs, float startSize) {
        Particle &particle = this->particles[particleEnd];
        particle.x = x;
        particle.y = y;
        particle.z = z;
        particle.xs = xs;
        particle.ys = ys;
        particle.zs = zs;
        particle.startSize = startSize;
        particle.life = 1.0f;

        particleEnd = (particleEnd + 1) % this->particles.size();
        if (particleEnd == particleStart) {
            particleStart = (particleStart + 1) % this->particles.size();
        }
    }

    void ParticleEmitter::emitIn(float x, float y, float z, float xs, float ys, float zs, float startSize, float timeToEmit) {
        Particle &particle = this->particles[particleEnd];
        particle.x = x;
        particle.y = y;
        particle.z = z;
        particle.xs = xs;
        particle.ys = ys;
        particle.zs = zs;
        particle.startSize = startSize;
        particle.life = 1.0f;

        queue.emplace(time+timeToEmit, particle);
    }

#define dbg(x) (#x " = ") << x << " | "

    void ParticleEmitter::onUpdate(float dt) {
        time += dt;
        while ((!queue.empty()) && queue.top().first <= time) {
            this->particles[particleEnd] = queue.top().second;
            queue.pop();
            particleEnd = (particleEnd + 1) % this->particles.size();
            if (particleEnd == particleStart) {
                particleStart = (particleStart + 1) % this->particles.size();
            }
        }

        for (unsigned long int i = this->particleStart; i != this->particleEnd; i = (i+1) % this->particles.size()) {
            Particle &particle = this->particles[i];

            float timeDelta = dt / this->proprieties.duration;
            particle.life -= timeDelta;

            if (particle.life <= 0.0f) {
                particleStart = (particleStart + 1) % this->particles.size();
                continue;
            }

            // Update position
            particle.xs += dt * this->proprieties.xa / 2.0f;
            particle.ys += dt * this->proprieties.ya / 2.0f;
            particle.zs += dt * this->proprieties.za / 2.0f;
            particle.x += dt * particle.xs;
            particle.y += dt * particle.ys;
            particle.z += dt * particle.zs;
        }
    }

    void ParticleEmitter::onRender(Renderer &renderer) {
        for (unsigned long int i = this->particleStart; i != this->particleEnd; i = (i+1) % this->particles.size()) {
            Particle &particle = this->particles[i];

            if (particle.life > 1.0f) {
                continue;
            }

            /*
            uint32_t seed = ((uint32_t) 0b1010101010101010101010101010101)+i;
            float rotationStart = _internal::generate_floats(seed);
            float rotationEnd = _internal::generate_floats(seed);
            float randomX = _internal::generate_floats(seed);
            float randomY = _internal::generate_floats(seed);
            float randomZ = _internal::generate_floats(seed);

            float positionSpread = 0.1f;
            float velocitySpread = 0.1f;
             */

            float t = (1.0f - particle.life) * this->proprieties.duration;

            float x = particle.x + (particle.xs * t) + (this->proprieties.xa * t * t * 0.5f);
            float y = particle.y + (particle.ys * t) + (this->proprieties.ya * t * t * 0.5f);
            float z = particle.z + (particle.zs * t) + (this->proprieties.za * t * t * 0.5f);

            float rotationX = this->proprieties.rotationSpeedX * t;
            float rotationY = this->proprieties.rotationSpeedX * t;
            float rotationZ = this->proprieties.rotationSpeedX * t;

            float size = (particle.startSize * (1.0f-(1.0f-particle.life))) + (this->proprieties.finalSize * (1.0f-particle.life));

            // Create tranformation matrix
            auto model = Matrix_Identity();
            auto translate = Matrix_Translate(x, y, z);
            auto rx = Matrix_Rotate_X(rotationX);
            auto ry = Matrix_Rotate_Y(rotationY);
            auto rz = Matrix_Rotate_Z(rotationZ);
            auto scale = Matrix_Scale(size, size, size);
            model *= translate;
            model *= rx;
            model *= ry;
            model *= rz;
            model *= scale;

            // Send transformation matrix to the GPU
            glUniformMatrix4fv(renderer.model, 1, GL_FALSE, glm::value_ptr(model));

            // Draw object
            this->proprieties.object.draw();
        }
    }
}
