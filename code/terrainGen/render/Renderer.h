#pragma once
#include "..\world\world.h"
#include "..\TerrainConfig.h"
#include "..\terrain\TerrainGenerator.h"
#include "..\terrain\RiverGenerator.h"


extern "C" { struct GLFWwindow; }

class Renderer {
public:
    bool init(World* world, TerrainGenerator* gen, RiverGenerator* rivers);
    void buildTexture(const World& world, const TerrainConfig& config);
    void run();
    void cleanup();

    // Scroll callback needs to be static for GLFW
    static void scrollCallback(GLFWwindow* window, double xoff, double yoff);

private:
    void drawUI();
    void regenerate();

    World* m_world = nullptr;
    TerrainGenerator* m_gen = nullptr;
    RiverGenerator* m_rivers = nullptr;
    TerrainConfig     m_config;

    // Zoom/pan state
    float m_zoom = 0.25f;
    float m_panX = 0.0f;
    float m_panY = 0.0f;
    bool  m_dragging = false;
    double m_lastMouseX = 0.0, m_lastMouseY = 0.0;

    void* m_window = nullptr;
    unsigned int m_texture = 0;
    unsigned int m_VAO = 0, m_VBO = 0, m_EBO = 0;
    unsigned int m_shaderProgram = 0;

    unsigned int compileShader(unsigned int type, const char* source);
    void updateQuad();
};