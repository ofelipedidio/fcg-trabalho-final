#include <random>

namespace Random
{
    std::mt19937 s_RandomEngine;
    std::uniform_real_distribution<float> s_Distribution;

    void Init()
    {
        s_RandomEngine.seed(std::random_device()());
        s_Distribution = std::uniform_real_distribution<float>(0.0f, 1.0f);
    }

    float Float()
    {
        return (float)s_Distribution(s_RandomEngine);
    }
}