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

void TerrainGenerator::generate(World& world, const TerrainConfig& config) {
    generateHeights(world, config);
    normalizeHeights(world);
    applyIslandMask(world, config);
    generateClimate(world, config);
    assignBiomes(world);
}

void TerrainGenerator::generateHeights(World& world, const TerrainConfig& config) {
    PerlinNoise heightNoise(rand());
    PerlinNoise moistureNoise(rand());
    PerlinNoise temperatureNoise(rand());

    for (int y = 0; y < world.getHeight(); ++y) {
        for (int x = 0; x < world.getWidth(); ++x) {
            float nx = (float)x / world.getWidth() * NOISE_SAMPLE_SCALE;
            float ny = (float)y / world.getHeight() * NOISE_SAMPLE_SCALE;

            float continents = heightNoise.fractalNoise(nx * config.continentScale, ny * config.continentScale, HEIGHT_CONTINENT_OCT, 2.0f, 0.5f);
            float mediumDetail = heightNoise.fractalNoise(nx * config.mediumScale, ny * config.mediumScale, HEIGHT_MEDIUM_OCT, 2.0f, 0.5f);
            float fineDetail = heightNoise.fractalNoise(nx * config.fineScale, ny * config.fineScale, HEIGHT_FINE_OCT, 2.0f, 0.4f);

            float height = continents * config.continentWeight
                + mediumDetail * config.mediumWeight
                + fineDetail * config.fineWeight;

            float cx = nx - 0.5f, cy = ny - 0.5f;
            float dist = std::sqrt(cx * cx + cy * cy);
            float islandMask = 1.0f - smoothstep(config.islandInnerRadius, config.islandOuterRadius, dist);
            height = height * (1.0f - config.islandBlend + config.islandBlend * islandMask);

            Tile& t = world.at(x, y);
            t.height = height;
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

void TerrainGenerator::applyIslandMask(World& world, const TerrainConfig& config) {
    for (int y = 0; y < world.getHeight(); ++y) {
        for (int x = 0; x < world.getWidth(); ++x) {
            float nx = (float)x / world.getWidth();
            float ny = (float)y / world.getHeight();
            float cx = nx - 0.5f, cy = ny - 0.5f;
            float dist = std::sqrt(cx * cx + cy * cy);
            float mask = 1.0f - smoothstep(
                config.islandInnerRadius,
                config.islandOuterRadius, dist);

            float h = world.at(x, y).height;
            // Lerp between original height and masked height
            world.at(x, y).height = h * (1.0f - config.islandBlend)
                + h * mask * config.islandBlend;
        }
    }
}

void TerrainGenerator::assignBiomes(World& world) {
    for (int y = 0; y < world.getHeight(); ++y)
        for (int x = 0; x < world.getWidth(); ++x) {
            Tile& t = world.at(x, y);
            t.biome = determineBiome(t.height, t.moisture, t.temperature);
        }
}

void TerrainGenerator::generateClimate(World& world, const TerrainConfig& config) {
    PerlinNoise moistureNoise(rand());
    PerlinNoise temperatureNoise(rand());

    for (int y = 0; y < world.getHeight(); ++y) {
        for (int x = 0; x < world.getWidth(); ++x) {
            float nx = (float)x / world.getWidth();
            float ny = (float)y / world.getHeight();

            Tile& t = world.at(x, y);

            // Now t.height is normalized, so near-water and elevation
            // cooling work against the actual final height values
            float baseMoisture = moistureNoise.fractalNoise(
                nx * config.moistureScale, ny * config.moistureScale,
                MOISTURE_OCTAVES, 2.1f, 0.5f);
            baseMoisture = (baseMoisture + 1.0f) / 2.0f;
            if (t.height < OCEAN_THRESHOLD + 0.05f)
                baseMoisture = std::min(1.0f, baseMoisture + MOISTURE_NEAR_WATER);
            t.moisture = clamp(baseMoisture, 0.0f, 1.0f);

            float tempNoise = temperatureNoise.fractalNoise(
                nx * config.tempScale, ny * config.tempScale,
                TEMP_OCTAVES, 2.0f, 0.5f);
            tempNoise = (tempNoise + 1.0f) / 2.0f;
            float elevationCooling = smoothstep(0.5f, 0.85f, t.height)
                * config.elevationCooling;
            t.temperature = clamp(tempNoise * 0.9f + 0.05f - elevationCooling,
                0.0f, 1.0f);
        }
    }
}