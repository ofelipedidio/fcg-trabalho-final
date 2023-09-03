#include "particle.h"
#include "matrices.h"

namespace Particle
{
    ParticleEmitter::ParticleEmitter(int maxParticleCount)
    {
        this->particles = std::vector<Particle>(maxParticleCount);
        this->particleStart = 0;
        this->particleEnd = 0;
    }

    void ParticleEmitter::emit(ParticleProprieties proprieties)
    {
        Particle &particle = this->particles[particleEnd];
        particle.props = proprieties;
        particle.life = 1.0f;

        particleEnd = (particleEnd + 1) % this->particles.size();
        if (particleEnd == particleStart)
        {
            particleStart = (particleStart + 1) % this->particles.size();
        }
    }

    void ParticleEmitter::emitIn(ParticleProprieties proprieties, float timeToEmit)
    {
        Particle &particle = this->particles[particleEnd];
        particle.props = proprieties;
        particle.life = 1.0f + timeToEmit;

        particleEnd = (particleEnd + 1) % this->particles.size();
        if (particleEnd == particleStart)
        {
            particleStart = (particleStart + 1) % this->particles.size();
        }
    }

#define dbg(x) (#x " = ") << x << " | "

    void ParticleEmitter::onUpdate(float dt)
    {
        for (unsigned long int i = this->particleStart; i != this->particleEnd; i = (i + 1) % this->particles.size())
        {
            Particle &particle = this->particles[i];

            float timeDelta = dt / particle.props.duration;
            if (particle.life > 1.0f)
            {
                particle.life -= dt;

                if (particle.life >= 1.0f)
                {
                    continue;
                }
                else
                {
                    timeDelta = (1.0f - particle.life) / particle.props.duration;
                    particle.life = 1.0f;
                }
            }

            particle.life -= timeDelta;

            if (particle.life <= 0.0f)
            {
                particleStart = (particleStart + 1) % this->particles.size();
                continue;
            }

            // Update position
            particle.props.xs += dt * particle.props.xa / 2.0f;
            particle.props.ys += dt * particle.props.ya / 2.0f;
            particle.props.zs += dt * particle.props.za / 2.0f;
            particle.props.x += dt * particle.props.xs;
            particle.props.y += dt * particle.props.ys;
            particle.props.z += dt * particle.props.zs;

            // Update rotation
            particle.props.rotationX += timeDelta * particle.props.rotationSpeedX;
            particle.props.rotationY += timeDelta * particle.props.rotationSpeedY;
            particle.props.rotationZ += timeDelta * particle.props.rotationSpeedZ;

            // Update size
            particle.props.size += timeDelta * particle.props.sizeChange;
        }
    }

    void ParticleEmitter::onRender(Renderer &renderer)
    {
        for (unsigned long int i = this->particleStart; i != this->particleEnd; i = (i + 1) % this->particles.size())
        {
            Particle &particle = this->particles[i];

            if (particle.life > 1.0f)
            {
                continue;
            }

            // Create tranformation matrix
            auto model = Matrix_Identity();
            auto translate = Matrix_Translate(particle.props.x, particle.props.y, particle.props.z);
            // auto rotate = Matrix_Rotate_XYZ(particle.props.rotationX, particle.props.rotationY, particle.props.rotationZ);
            auto rx = Matrix_Rotate_X(particle.props.rotationX);
            auto ry = Matrix_Rotate_Y(particle.props.rotationY);
            auto rz = Matrix_Rotate_Z(particle.props.rotationZ);
            auto scale = Matrix_Scale(particle.props.size, particle.props.size, particle.props.size);
            model *= translate;
            model *= rx;
            model *= ry;
            model *= rz;
            model *= scale;
            // model *= rotate;

            // Send transformation matrix to the GPU
            glUniformMatrix4fv(renderer.model, 1, GL_FALSE, glm::value_ptr(model));

            // Draw object
            particle.props.object.draw();
        }
    }
}
