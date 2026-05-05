#include "BiomeUtils.h"
#include "..\Constants.h"

Biome determineBiome(float height, float moisture, float temperature) {
    if (height < OCEAN_THRESHOLD)
        return Biome::Ocean;
    if (height < BEACH_THRESHOLD)
        return Biome::Beach;
    if (height > MOUNTAIN_THRESHOLD) {
        if (height > SNOW_THRESHOLD)
            return Biome::Tundra;
        return Biome::Mountain;
    }
    if (temperature < COLD_TEMP)
        return Biome::Tundra;
    if (temperature < COOL_TEMP) {
        if (moisture > WET_MOISTURE)
            return Biome::Forest;
        return Biome::Plains;
    }
    if (temperature < WARM_TEMP) {
        if (moisture > VERY_WET_MOISTURE)
            return Biome::Forest;
        return Biome::Plains;
    }
    if (moisture < DRY_MOISTURE)
        return Biome::Desert;
    if (moisture < MOIST_MOISTURE)
        return Biome::Plains;
    return Biome::Forest;
}

void biomeToColor(Biome biome, unsigned char& r, unsigned char& g, unsigned char& b) {
    switch (biome) {
    case Biome::Ocean:    r = 25;  g = 60;  b = 140; break;
    case Biome::Beach:    r = 220; g = 205; b = 150; break;
    case Biome::Plains:   r = 100; g = 165; b = 80;  break;
    case Biome::Forest:   r = 30;  g = 105; b = 50;  break;
    case Biome::Desert:   r = 210; g = 180; b = 100; break;
    case Biome::Tundra:   r = 210; g = 225; b = 230; break;
    case Biome::Mountain: r = 110; g = 100; b = 90;  break;
    }
}