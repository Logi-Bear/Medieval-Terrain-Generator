#include "Renderer.h"
#include "..\terrain\BiomeUtils.h"
#include "..\Constants.h"
#include <ew/external/opengl/include/glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <cstdlib>

// Static pointer so scroll callback can reach the instance
static Renderer* g_renderer = nullptr;

void Renderer::scrollCallback(GLFWwindow*, double, double yoff) {
    if (!g_renderer) return;
    // Don't zoom if ImGui is capturing the mouse
    if (ImGui::GetIO().WantCaptureMouse) return;
    g_renderer->m_zoom *= (yoff > 0) ? 0.9f : 1.1f;
    g_renderer->m_zoom = std::max(0.1f, std::min(g_renderer->m_zoom, 10.0f));
    g_renderer->updateQuad();
}

bool Renderer::init(World* world, TerrainGenerator* gen, RiverGenerator* rivers) {
    m_world = world;
    m_gen = gen;
    m_rivers = rivers;

    g_renderer = this;

    glfwSetErrorCallback([](int, const char* desc) {
        std::cerr << "GLFW Error: " << desc << "\n";
        });

    if (!glfwInit()) { std::cerr << "Failed to init GLFW\n"; return false; }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    m_window = glfwCreateWindow(1100, 800, "DND Map Generator", nullptr, nullptr);
    if (!m_window) { std::cerr << "Failed to create window\n"; glfwTerminate(); return false; }

    GLFWwindow* win = (GLFWwindow*)m_window;
    glfwMakeContextCurrent(win);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to init GLAD\n"; return false;
    }

    glfwSetScrollCallback(win, scrollCallback);
    glViewport(0, 0, 1100, 800);

    // ImGui setup
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(win, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Quad
    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glGenBuffers(1, &m_EBO);

    unsigned int indices[] = { 0, 1, 2, 2, 3, 0 };
    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, 16 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    updateQuad();

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

void Renderer::updateQuad() {
    // Map viewport is left 800px, panel is right 300px
    // NDC range for map area: x in [-1, 0.45], y in [-1, 1]
    float mapRight = 0.45f; // leaves room for 300px panel on 1100px window

    float hw = m_zoom;
    float hh = m_zoom;

    float u0 = 0.5f + m_panX - hw * 0.5f;
    float u1 = 0.5f + m_panX + hw * 0.5f;
    float v0 = 0.5f + m_panY - hh * 0.5f;
    float v1 = 0.5f + m_panY + hh * 0.5f;

    float vertices[] = {
        -1.0f,    -1.0f,  u0, v1,
         mapRight,-1.0f,  u1, v1,
         mapRight, 1.0f,  u1, v0,
        -1.0f,    1.0f,   u0, v0,
    };

    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
}

void Renderer::buildTexture(const World& world, const TerrainConfig& config) {
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

            float nX = (world.at(xL, y).height - world.at(xR, y).height) * config.slopeScale;
            float nZ = (world.at(x, yD).height - world.at(x, yU).height) * config.slopeScale;
            float nY = 2.0f;
            float nLen = std::sqrt(nX * nX + nY * nY + nZ * nZ);
            nX /= nLen; nY /= nLen; nZ /= nLen;

            float diffuse = std::max(nX * lightX + nY * lightY + nZ * lightZ, 0.0f);
            float lighting = config.ambient + (1.0f - config.ambient) * diffuse;

            const Tile& t = world.at(x, y);
            if (t.riverStrength > 0.0f) {
                r = 25; g = 60; b = 140;
            }

            int idx = (y * MAP_WIDTH + x) * 3;
            pixels[idx + 0] = (unsigned char)(std::min(r * lighting * BRIGHTNESS, 255.0f));
            pixels[idx + 1] = (unsigned char)(std::min(g * lighting * BRIGHTNESS, 255.0f));
            pixels[idx + 2] = (unsigned char)(std::min(b * lighting * BRIGHTNESS, 255.0f));
        }
    }

    if (m_texture == 0) glGenTextures(1, &m_texture);
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, MAP_WIDTH, MAP_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
}

void Renderer::regenerate() {
    std::srand(std::rand());
    m_gen->generate(*m_world, m_config);
    m_rivers->reset();
    m_rivers->generate(1.0f, 0.01f);
    buildTexture(*m_world, m_config);
}

void Renderer::drawUI() {
    ImGui::SetNextWindowPos(ImVec2(800, 0));
    ImGui::SetNextWindowSize(ImVec2(300, 800));
    ImGui::Begin("Settings", nullptr,
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

    if (ImGui::Button("Regenerate", ImVec2(-1, 30))) regenerate();

    ImGui::Separator();
    if (ImGui::CollapsingHeader("Height Layers", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("Continent");
        ImGui::SliderFloat("Scale##c", &m_config.continentScale, 0.5f, 8.0f);
        ImGui::SliderFloat("Weight##c", &m_config.continentWeight, 0.0f, 1.0f);
        ImGui::Spacing();
        ImGui::Text("Medium");
        ImGui::SliderFloat("Scale##m", &m_config.mediumScale, 1.0f, 15.0f);
        ImGui::SliderFloat("Weight##m", &m_config.mediumWeight, 0.0f, 1.0f);
        ImGui::Spacing();
        ImGui::Text("Fine");
        ImGui::SliderFloat("Scale##f", &m_config.fineScale, 2.0f, 30.0f);
        ImGui::SliderFloat("Weight##f", &m_config.fineWeight, 0.0f, 1.0f);
    }

    ImGui::Separator();
    if (ImGui::CollapsingHeader("Island Shape")) {
        ImGui::SliderFloat("Inner Radius", &m_config.islandInnerRadius, 0.05f, 0.45f);
        ImGui::SliderFloat("Outer Radius", &m_config.islandOuterRadius, 0.1f, 0.7f);
        ImGui::SliderFloat("Blend", &m_config.islandBlend, 0.0f, 1.0f);
    }

    ImGui::Separator();
    if (ImGui::CollapsingHeader("Climate")) {
        ImGui::SliderFloat("Moisture Scale", &m_config.moistureScale, 0.5f, 8.0f);
        ImGui::SliderFloat("Temp Scale", &m_config.tempScale, 0.5f, 8.0f);
        ImGui::SliderFloat("Elev. Cooling", &m_config.elevationCooling, 0.0f, 1.0f);
    }

    ImGui::Separator();
    if (ImGui::CollapsingHeader("Shading")) {
        ImGui::SliderFloat("Slope Scale", &m_config.slopeScale, 0.0f, 20.0f);
        ImGui::SliderFloat("Ambient", &m_config.ambient, 0.0f, 1.0f);
        if (ImGui::Button("Rebuild Shading", ImVec2(-1, 0)))
            buildTexture(*m_world, m_config);
        ImGui::TextDisabled("(no regen needed)");
    }

    ImGui::Separator();
    if (ImGui::CollapsingHeader("Zoom/Pan")) {
        ImGui::SliderFloat("Zoom", &m_zoom, 0.1f, 10.0f);
        ImGui::SliderFloat("Pan X", &m_panX, -0.5f, 0.5f);
        ImGui::SliderFloat("Pan Y", &m_panY, -0.5f, 0.5f);
        if (ImGui::IsItemEdited()) updateQuad();
        if (ImGui::Button("Reset View", ImVec2(-1, 0))) {
            m_zoom = 1.0f; m_panX = 0.0f; m_panY = 0.0f;
            updateQuad();
        }
    }

    ImGui::End();

    // Update quad if zoom sliders changed
    if (ImGui::IsItemEdited()) updateQuad();
}

void Renderer::run() {
    GLFWwindow* win = (GLFWwindow*)m_window;

    while (!glfwWindowShouldClose(win)) {
        glfwPollEvents();

        // Pan with middle mouse drag
        if (!ImGui::GetIO().WantCaptureMouse) {
            if (glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS) {
                double mx, my;
                glfwGetCursorPos(win, &mx, &my);
                if (m_dragging) {
                    m_panX -= (float)(mx - m_lastMouseX) / 800.0f * m_zoom;
                    m_panY -= (float)(my - m_lastMouseY) / 800.0f * m_zoom;
                    updateQuad();
                }
                m_dragging = true;
                m_lastMouseX = mx; m_lastMouseY = my;
            }
            else {
                m_dragging = false;
            }
        }

        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(m_shaderProgram);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_texture);
        glBindVertexArray(m_VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        drawUI();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(win);
    }
}

void Renderer::cleanup() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glDeleteVertexArrays(1, &m_VAO);
    glDeleteBuffers(1, &m_VBO);
    glDeleteBuffers(1, &m_EBO);
    glDeleteProgram(m_shaderProgram);
    glDeleteTextures(1, &m_texture);
    glfwDestroyWindow((GLFWwindow*)m_window);
    glfwTerminate();
}

unsigned int Renderer::compileShader(unsigned int type, const char* source) {
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