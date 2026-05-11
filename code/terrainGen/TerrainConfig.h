#pragma once
#include "Constants.h"

struct TerrainConfig {
    // Height layers
    float continentScale = HEIGHT_CONTINENT_SCALE;
    float mediumScale = HEIGHT_MEDIUM_SCALE;
    float fineScale = HEIGHT_FINE_SCALE;
    float continentWeight = HEIGHT_CONTINENT_WEIGHT;
    float mediumWeight = HEIGHT_MEDIUM_WEIGHT;
    float fineWeight = HEIGHT_FINE_WEIGHT;

    // Moisture
    float moistureScale = MOISTURE_SCALE;

    // Temperature
    float tempScale = TEMP_SCALE;
    float elevationCooling = TEMP_ELEVATION_COOLING;

    // Island shape
    float islandInnerRadius = ISLAND_INNER_RADIUS;
    float islandOuterRadius = ISLAND_OUTER_RADIUS;
    float islandBlend = ISLAND_BLEND;

    // Lighting
    float slopeScale = SLOPE_SCALE;
    float ambient = AMBIENT;
};