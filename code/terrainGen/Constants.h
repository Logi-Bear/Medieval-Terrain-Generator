#pragma once

// Map size
const int MAP_WIDTH = 256;
const int MAP_HEIGHT = 256;

// --- Noise Parameters ---
const float HEIGHT_CONTINENT_SCALE = 2.2f;
const float HEIGHT_MEDIUM_SCALE = 5.0f;
const float HEIGHT_FINE_SCALE = 12.0f;
const int   HEIGHT_CONTINENT_OCT = 3;
const int   HEIGHT_MEDIUM_OCT = 4;
const int   HEIGHT_FINE_OCT = 3;
const float HEIGHT_CONTINENT_WEIGHT = 0.55f;
const float HEIGHT_MEDIUM_WEIGHT = 0.30f;
const float HEIGHT_FINE_WEIGHT = 0.15f;

const float MOISTURE_SCALE = 3.5f;
const int   MOISTURE_OCTAVES = 4;
const float MOISTURE_NEAR_WATER = 0.1f;

const float TEMP_SCALE = 2.8f;
const int   TEMP_OCTAVES = 4;
const float TEMP_ELEVATION_COOLING = 0.35f;

// --- Island Shape ---
const float ISLAND_INNER_RADIUS = 0.25f;
const float ISLAND_OUTER_RADIUS = 0.48f;
const float ISLAND_BLEND = 0.70f;

// --- Biome Thresholds ---
const float OCEAN_THRESHOLD = 0.40f;
const float BEACH_THRESHOLD = 0.47f;
const float MOUNTAIN_THRESHOLD = 0.70f;
const float SNOW_THRESHOLD = 0.80f;
const float COLD_TEMP = 0.25f;
const float COOL_TEMP = 0.45f;
const float WARM_TEMP = 0.65f;
const float DRY_MOISTURE = 0.35f;
const float MOIST_MOISTURE = 0.50f;
const float WET_MOISTURE = 0.60f;
const float VERY_WET_MOISTURE = 0.75f;

// --- Lighting ---
const float LIGHT_X = 1.0f;
const float LIGHT_Y = 2.0f;
const float LIGHT_Z = 1.0f;
const float SLOPE_SCALE = 12.0f;
const float AMBIENT = 0.01f;