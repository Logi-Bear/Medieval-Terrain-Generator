#include <cstdlib>
#include <ctime>
#include "Constants.h"
#include "world/world.h"
#include "terrain/TerrainGenerator.h"
#include "TerrainConfig.h"
#include "render/Renderer.h"
#include "terrain/RiverGenerator.h"

int main() {
    std::srand(std::time(0));

    World world(MAP_WIDTH, MAP_HEIGHT);
    TerrainGenerator gen;
    TerrainConfig config;

    gen.generate(world, config);

    RiverGenerator rivers(&world);
    rivers.generate(1.0f, 0.01f);

    Renderer renderer;
    if (!renderer.init(&world, &gen, &rivers)) return -1;
    renderer.buildTexture(world, config);
    renderer.run();
    renderer.cleanup();

    return 0;
}