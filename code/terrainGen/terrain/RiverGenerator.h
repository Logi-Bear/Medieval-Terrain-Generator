#pragma once
#include "../world/World.h"

class RiverGenerator {
public:
    RiverGenerator(World* world);
    void generate(float frequency = 3.0f, float threshold = 0.05f);
    void reset();

private:
    World* m_world;
};