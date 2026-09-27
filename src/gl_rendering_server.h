#pragma once
#include "rendering_server.h"
#include "shader.h"
#include <GL/glew.h>
#include <memory>
#include <vector>

struct BatchVertex {
    float x, y;
    float u, v;
    float r, g, b, a;
};

class GLRenderingServer : public RenderingServer {
public:
    GLRenderingServer(int width, int height);
    ~GLRenderingServer() override;

    void clear_screen(const Color& c) override;
    void flush() override;
    TextureID white_texture() const override { return m_whiteTexture; }
    void set_viewport(int w, int h) override;

    TextureID create_texture(const unsigned char* pixels, int w, int h);

private:
    void emit_batch(TextureID texture);
    void push_quad_vertices(const DrawItem& item);
    void build_projection(int w, int h);

    GLuint m_vao = 0, m_vbo = 0;
    GLuint m_whiteTexture = 0;
    std::unique_ptr<Shader> m_shader;
    std::vector<BatchVertex> m_vertices;
    float m_projection[16];
    size_t m_capacity = 0;      // current VBO capacity in vertices
};