#include "framebuffer.h"
#include <iostream>

Framebuffer::Framebuffer(int width, int height)
    : m_width(width > 0 ? width : 1), m_height(height > 0 ? height : 1) {
    create();
}

Framebuffer::~Framebuffer() { destroy(); }

void Framebuffer::destroy() {
    if (m_colorTex) { glDeleteTextures(1, &m_colorTex); m_colorTex = 0; }
    if (m_fbo)      { glDeleteFramebuffers(1, &m_fbo);  m_fbo = 0; }
}

void Framebuffer:create() {
    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    glGenTextures(1, &m_colorTex);
    glBindTexture(GL_TEXTURE_2D, m_colorTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, m_colorTex, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Framebuffer incomplete!" << std::endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::resize(int width, int height) {
    if (width < 1) width = 1;
    if (height < 1) height = 1;
    if (width == m_width && height == m_height) return;
    m_width = width; m_height = height;
    destroy();
    create();
}

void Framebuffer::bind() {
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glViewport(0, 0, m_width, m_height);
}

void Framebuffer::unbind() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
