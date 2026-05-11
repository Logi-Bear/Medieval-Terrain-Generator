#include "RiverGenerator.h"
#include "../Constants.h"
#include "../noise/PerlinNoise.h"
#include <cstdlib>
#include <cmath>
#include <algorithm>

RiverGenerator::RiverGenerator(World* world)
    : m_world(world)
{
}

void RiverGenerator::reset() {
    for (int y = 0; y < m_world->getHeight(); ++y)
        for (int x = 0; x < m_world->getWidth(); ++x) {
            m_world->at(x, y).riverStrength = 0.0f;
            m_world->at(x, y).isLake = false;
        }
}

void RiverGenerator::generate(float frequency, float threshold) {
    int width = m_world->getWidth();
    int height = m_world->getHeight();

    PerlinNoise riverNoise(rand());

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            Tile& t = m_world->at(x, y);

            // No rivers in ocean or beach
            if (t.biome == Biome::Ocean) continue;

            // No rivers on high mountains
            if (t.height > MOUNTAIN_THRESHOLD) continue;

            float nx = (float)x / width * frequency;
            float ny = (float)y / height * frequency;

            // Sample noise and take absolute value — 
            // river is where noise crosses zero (the narrow band near 0)
            float n = riverNoise.fractalNoise(nx, ny, 4, 2.0f, 0.5f);
            float riverValue = std::abs(n);

            if (riverValue < threshold) {
                // Strength is stronger closer to center of the band
                t.riverStrength = 1.0f - (riverValue / threshold);
            }
        }
    }
}