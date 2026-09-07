#include "renderer.h"
#include <cstring>

static const char* kVertexShader = R"(
#version 330 core
layout(location = 0) in vec2 aPos;
uniform mat4 uProjection;
uniform mat4 uModel;
void main() {
    gl_Position = uProjection * uModel * vec4(aPos, 0.0, 1.0);
}
)";

static const char* kFragmentShader = R"(
#version 330 core
out vec4 FragColor;
uniform vec4 uColor;
void main() {
    FragColor = uColor;
}
)";

static void mat4Identity(float* m) {
    std::memset(m, 0, sizeof(float) * 16);
    m[0] = m[5] = m[10] = m[15] = 1.0f;
}

static void mat4Ortho(float* m, float left, float right, float bottom, float top) {
    mat4Identity(m);
    m[0] = 2.0f / (right - left);
    m[5] = 2.0f / (top - bottom);
    m[10] = -1.0f;
    m[12] = -(right + left) / (right - left);
    m[13] = -(top + bottom) / (top - bottom);
}

static void mat4TranslateScale(float* m, float x, float y, float w, float h) {
    mat4Identity(m);
    m[0] = w;
    m[5] = h;
    m[12] = x;
    m[13] = y;
}

void Renderer2D::buildOrtho(int width, int height) {
    // Origin top-left, y grows downward — common convention for 2D engines.
    mat4Ortho(m_projection, 0.0f, (float)width, (float)height, 0.0f);
}

Renderer2D::Renderer2D(int screenWidth, int screenHeight) {
    buildOrtho(screenWidth, screenHeight);
    m_shader = std::make_unique<Shader>(kVertexShader, kFragmentShader);

    float vertices[] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,
    };

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}

Renderer2D::~Renderer2D() {
    glDeleteBuffers(1, &m_vbo);
    glDeleteVertexArrays(1, &m_vao);
}

void Renderer2D::resize(int width, int height) {
    buildOrtho(width, height);
    glViewport(0, 0, width, height);
}

void Renderer2D::beginFrame(float r, float g, float b, float a) {
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer2D::drawQuad(float x, float y, float w, float h,
                           float r, float g, float b, float a) {
    m_shader->use();
    m_shader->setMat4("uProjection", m_projection);

    float model[16];
    mat4TranslateScale(model, x, y, w, h);
    m_shader->setMat4("uModel", model);
    m_shader->setVec4("uColor", r, g, b, a);

    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}