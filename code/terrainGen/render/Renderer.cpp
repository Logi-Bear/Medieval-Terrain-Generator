#include "Renderer.h"
#include "..\terrain\BiomeUtils.h"
#include "..\Constants.h"
#include <ew/external/opengl/include/glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

bool Renderer::init() {
    glfwSetErrorCallback([](int, const char* desc) {
        std::cerr << "GLFW Error: " << desc << "\n";
        });

    if (!glfwInit()) { std::cerr << "Failed to init GLFW\n"; return false; }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    m_window = glfwCreateWindow(800, 800, "DND Map Generator", nullptr, nullptr);
    if (!m_window) { std::cerr << "Failed to create window\n"; glfwTerminate(); return false; }

    glfwMakeContextCurrent((GLFWwindow*)m_window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to init GLAD\n"; return false;
    }

    glViewport(0, 0, 800, 800);

    // Quad
    float vertices[] = {
        -1.0f, -1.0f, 0.0f, 0.0f,
         1.0f, -1.0f, 1.0f, 0.0f,
         1.0f,  1.0f, 1.0f, 1.0f,
        -1.0f,  1.0f, 0.0f, 1.0f
    };
    unsigned int indices[] = { 0, 1, 2, 2, 3, 0 };

    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glGenBuffers(1, &m_EBO);
    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    // Shaders
    const char* vert = R"glsl(
        #version 330 core
        layout(location = 0) in vec2 aPos;
        layout(location = 1) in vec2 aTex;
        out vec2 TexCoord;
        void main() { TexCoord = aTex; gl_Position = vec4(aPos, 0.0, 1.0); }
    )glsl";

    const char* frag = R"glsl(
        #version 330 core
        out vec4 FragColor;
        in vec2 TexCoord;
        uniform sampler2D mapTexture;
        void main() { FragColor = texture(mapTexture, TexCoord); }
    )glsl";

    GLuint vs = compileShader(GL_VERTEX_SHADER, vert);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, frag);
    m_shaderProgram = glCreateProgram();
    glAttachShader(m_shaderProgram, vs);
    glAttachShader(m_shaderProgram, fs);
    glLinkProgram(m_shaderProgram);
    glDeleteShader(vs);
    glDeleteShader(fs);

    glUseProgram(m_shaderProgram);
    glUniform1i(glGetUniformLocation(m_shaderProgram, "mapTexture"), 0);

    return true;
}

void Renderer::buildTexture(const World& world) {
    float lightX = LIGHT_X, lightY = LIGHT_Y, lightZ = LIGHT_Z;
    float lightLen = std::sqrt(lightX * lightX + lightY * lightY + lightZ * lightZ);
    lightX /= lightLen; lightY /= lightLen; lightZ /= lightLen;

    std::vector<unsigned char> pixels(MAP_WIDTH * MAP_HEIGHT * 3);

    for (int y = 0; y < MAP_HEIGHT; ++y) {
        for (int x = 0; x < MAP_WIDTH; ++x) {
            unsigned char r, g, b;
            biomeToColor(world.at(x, y).biome, r, g, b);

            int xL = std::max(x - 1, 0), xR = std::min(x + 1, MAP_WIDTH - 1);
            int yD = std::max(y - 1, 0), yU = std::min(y + 1, MAP_HEIGHT - 1);

            float nX = (world.at(xL, y).height - world.at(xR, y).height) * SLOPE_SCALE;
            float nZ = (world.at(x, yD).height - world.at(x, yU).height) * SLOPE_SCALE;
            float nY = 2.0f;
            float nLen = std::sqrt(nX * nX + nY * nY + nZ * nZ);
            nX /= nLen; nY /= nLen; nZ /= nLen;

            float diffuse = std::max(nX * lightX + nY * lightY + nZ * lightZ, 0.0f);
            float lighting = AMBIENT + (1.0f - AMBIENT) * diffuse;

            int idx = (y * MAP_WIDTH + x) * 3;
            pixels[idx + 0] = (unsigned char)(std::min(r * lighting, 255.0f));
            pixels[idx + 1] = (unsigned char)(std::min(g * lighting, 255.0f));
            pixels[idx + 2] = (unsigned char)(std::min(b * lighting, 255.0f));
        }
    }

    glGenTextures(1, &m_texture);
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, MAP_WIDTH, MAP_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

void Renderer::run() {
    while (!glfwWindowShouldClose((GLFWwindow*)m_window)) {
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_texture);
        glBindVertexArray(m_VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glfwSwapBuffers((GLFWwindow*)m_window);
    }
}

void Renderer::cleanup() {
    glDeleteVertexArrays(1, &m_VAO);
    glDeleteBuffers(1, &m_VBO);
    glDeleteBuffers(1, &m_EBO);
    glDeleteProgram(m_shaderProgram);
    glDeleteTextures(1, &m_texture);
    glfwDestroyWindow((GLFWwindow*)m_window);
    glfwTerminate();
}

GLuint Renderer::compileShader(unsigned int type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info[512];
        glGetShaderInfoLog(shader, 512, nullptr, info);
        std::cerr << "Shader error: " << info << "\n";
    }
    return shader;
}