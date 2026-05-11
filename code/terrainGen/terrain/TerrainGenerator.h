#pragma once
#include "../world/world.h"
#include "../TerrainConfig.h"

class TerrainGenerator {
public:
    void generate(World& world, const TerrainConfig& config);
private:
    void generateHeights(World& world, const TerrainConfig& config);
    void normalizeHeights(World& world);
    void assignBiomes(World& world);
    void generateClimate(World& world, const TerrainConfig& config);
    void applyIslandMask(World& world, const TerrainConfig& config);

    static float clamp(float x, float min, float max);
    static float smoothstep(float edge0, float edge1, float x);
};