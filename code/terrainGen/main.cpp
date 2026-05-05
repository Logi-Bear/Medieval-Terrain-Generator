#include <cstdlib>
#include <ctime>
#include "Constants.h"
#include "world/world.h"
#include "terrain/TerrainGenerator.h"
#include "render/Renderer.h"

int main() {
    std::srand(std::time(0));

    World world(MAP_WIDTH, MAP_HEIGHT);

    TerrainGenerator gen;
    gen.generate(world);

    Renderer renderer;
    if (!renderer.init()) return -1;
    renderer.buildTexture(world);
    renderer.run();
    renderer.cleanup();

    return 0;
}