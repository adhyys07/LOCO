#pragma once
#include <GL/glew.h>

// Offscreen color target. The editor renders the game into one of these so
// the result can be displayed inside the Viewport panel instead of the window.
class Framebuffer {
public:
    Framebuffer(int width, int height);
    ~Framebuffer();

    Framebuffer(const Framebuffer&) = delete;
    Framebuffer& operator=(const Framebuffer&) = delete;

    // No-op when the size is unchanged, so it is safe to call every frame.
    void resize(int width, int height);

    void bind();
    void unbind();

    GLuint color_texture() const { return m_colorTex; }
    int width() const  { return m_width; }
    int height() const { return m_height; }

private:
    void create();
    void destroy();

    GLuint m_fbo = 0;
    GLuint m_colorTex = 0;
    int m_width = 1, m_height = 1;
};
