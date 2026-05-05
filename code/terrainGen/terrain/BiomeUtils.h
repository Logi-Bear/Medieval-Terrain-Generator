#pragma once
#include "..\world\tile.h"

Biome determineBiome(float height, float moisture, float temperature);
void  biomeToColor(Biome biome, unsigned char& r, unsigned char& g, unsigned char& b);