#pragma once
#include "..\world\world.h"

class TerrainGenerator {
public:
    void generate(World& world);
private:
    void generateHeights(World& world);
    void normalizeHeights(World& world);
    void assignBiomes(World& world);

    static float clamp(float x, float min, float max);
    static float smoothstep(float edge0, float edge1, float x);
};