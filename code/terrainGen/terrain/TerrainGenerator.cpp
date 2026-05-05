#include "TerrainGenerator.h"
#include "BiomeUtils.h"
#include "..\Constants.h"
#include "..\noise\PerlinNoise.h"
#include <cmath>
#include <algorithm>
#include <cstdlib>

float TerrainGenerator::clamp(float x, float min, float max) {
    if (x < min) return min;
    if (x > max) return max;
    return x;
}

float TerrainGenerator::smoothstep(float edge0, float edge1, float x) {
    x = clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return x * x * (3.0f - 2.0f * x);
}

void TerrainGenerator::generate(World& world) {
    generateHeights(world);
    normalizeHeights(world);
    assignBiomes(world);
}

void TerrainGenerator::generateHeights(World& world) {
    PerlinNoise heightNoise(rand());
    PerlinNoise moistureNoise(rand());
    PerlinNoise temperatureNoise(rand());

    for (int y = 0; y < world.getHeight(); ++y) {
        for (int x = 0; x < world.getWidth(); ++x) {
            float nx = (float)x / world.getWidth();
            float ny = (float)y / world.getHeight();

            // Height
            float continents = heightNoise.fractalNoise(nx * HEIGHT_CONTINENT_SCALE, ny * HEIGHT_CONTINENT_SCALE, HEIGHT_CONTINENT_OCT, 2.0f, 0.5f);
            float mediumDetail = heightNoise.fractalNoise(nx * HEIGHT_MEDIUM_SCALE, ny * HEIGHT_MEDIUM_SCALE, HEIGHT_MEDIUM_OCT, 2.0f, 0.5f);
            float fineDetail = heightNoise.fractalNoise(nx * HEIGHT_FINE_SCALE, ny * HEIGHT_FINE_SCALE, HEIGHT_FINE_OCT, 2.0f, 0.4f);

            float height = continents * HEIGHT_CONTINENT_WEIGHT
                + mediumDetail * HEIGHT_MEDIUM_WEIGHT
                + fineDetail * HEIGHT_FINE_WEIGHT;

            // Island mask
            float cx = nx - 0.5f, cy = ny - 0.5f;
            float dist = std::sqrt(cx * cx + cy * cy);
            float islandMask = 1.0f - smoothstep(ISLAND_INNER_RADIUS, ISLAND_OUTER_RADIUS, dist);
            height = height * (1.0f - ISLAND_BLEND + ISLAND_BLEND * islandMask);

            // Moisture
            float baseMoisture = moistureNoise.fractalNoise(nx * MOISTURE_SCALE, ny * MOISTURE_SCALE, MOISTURE_OCTAVES, 2.1f, 0.5f);
            baseMoisture = (baseMoisture + 1.0f) / 2.0f;
            if (height < 0.45f)
                baseMoisture = std::min(1.0f, baseMoisture + MOISTURE_NEAR_WATER);
            float moisture = clamp(baseMoisture, 0.0f, 1.0f);

            // Temperature
            float tempNoise = temperatureNoise.fractalNoise(nx * TEMP_SCALE, ny * TEMP_SCALE, TEMP_OCTAVES, 2.0f, 0.5f);
            tempNoise = (tempNoise + 1.0f) / 2.0f;
            float elevationCooling = smoothstep(0.5f, 0.85f, height) * TEMP_ELEVATION_COOLING;
            float temperature = clamp(tempNoise * 0.9f + 0.05f - elevationCooling, 0.0f, 1.0f);

            Tile& t = world.at(x, y);
            t.height = height;
            t.moisture = moisture;
            t.temperature = temperature;
        }
    }
}

void TerrainGenerator::normalizeHeights(World& world) {
    float minH = 1.0f, maxH = -1.0f;
    for (int y = 0; y < world.getHeight(); ++y)
        for (int x = 0; x < world.getWidth(); ++x) {
            float h = world.at(x, y).height;
            minH = std::min(minH, h);
            maxH = std::max(maxH, h);
        }

    float range = maxH - minH;
    for (int y = 0; y < world.getHeight(); ++y)
        for (int x = 0; x < world.getWidth(); ++x)
            world.at(x, y).height = (world.at(x, y).height - minH) / range;
}

void TerrainGenerator::assignBiomes(World& world) {
    for (int y = 0; y < world.getHeight(); ++y)
        for (int x = 0; x < world.getWidth(); ++x) {
            Tile& t = world.at(x, y);
            t.biome = determineBiome(t.height, t.moisture, t.temperature);
        }
}