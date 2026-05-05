#pragma once
#include "..\world\world.h"

class Renderer {
public:
    bool init();
    void buildTexture(const World& world);
    void run();
    void cleanup();
private:
    void* m_window = nullptr;
    unsigned int m_texture = 0;
    unsigned int m_VAO = 0, m_VBO = 0, m_EBO = 0;
    unsigned int m_shaderProgram = 0;

    unsigned int compileShader(unsigned int type, const char* source);
};