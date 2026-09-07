#pragma once
#include <GL/glew.h>
#include <memory>
#include "shader.h"

// Renderer2D: draws axis-aligned colored quads.
// This is the seed of the engine's rendering system — next steps are
// textured quads (sprites), batching multiple quads into one draw call,
// and a camera/view matrix separate from the projection matrix.
class Renderer2D {
public:
    Renderer2D(int screenWidth, int screenHeight);
    ~Renderer2D();

    void beginFrame(float r, float g, float b, float a);
    void drawQuad(float x, float y, float w, float h,
                  float r, float g, float b, float a);
    void resize(int width, int height);

private:
    GLuint m_vao = 0, m_vbo = 0;
    std::unique_ptr<Shader> m_shader;
    float m_projection[16];

    void buildOrtho(int width, int height);
};
