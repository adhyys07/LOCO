#include "gl_rendering_server.h"
#include <algorithm>
#include <cstring>

static const char* kVS = R"(
#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUV;
layout(location = 2) in vec4 aColor;
uniform mat4 uProjection;
out vec2 vUV;
out vec4 vColor;
void main() {
    vUV = aUV;
    vColor = aColor;
    gl_Position = uProjection * vec4(aPos, 0.0, 1.0);
}
)";

static const char* kFS = R"(
#version 330 core
in vec2 vUV;
in vec4 vColor;
out vec4 FragColor;
uniform sampler2D uTexture;
void main() {
    FragColor = texture(uTexture, vUV) * vColor;
}
)";

void GLRenderingServer::build_projection(int w, int h) {
    std::memset(m_projection, 0, sizeof(m_projection));
    // Orthographic, origin top-left, y down.
    m_projection[0]  = 2.0f / (float)w;
    m_projection[5]  = -2.0f / (float)h;
    m_projection[10] = -1.0f;
    m_projection[12] = -1.0f;
    m_projection[13] = 1.0f;
    m_projection[15] = 1.0f;
}

GLRenderingServer::GLRenderingServer(int width, int height) {
    build_projection(width, height);
    m_shader = std::make_unique<Shader>(kVS, kFS);

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(BatchVertex),
                          (void*)offsetof(BatchVertex, x));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(BatchVertex),
                          (void*)offsetof(BatchVertex, u));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(BatchVertex),
                          (void*)offsetof(BatchVertex, r));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);

    // 1x1 white texture: lets untextured colored quads go through the
    // exact same batching path as real sprites. No branching in shader.
    unsigned char white[4] = {255, 255, 255, 255};
    m_whiteTexture = create_texture(white, 1, 1);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

GLRenderingServer::~GLRenderingServer() {
    glDeleteBuffers(1, &m_vbo);
    glDeleteVertexArrays(1, &m_vao);
    glDeleteTextures(1, &m_whiteTexture);
}

TextureID GLRenderingServer::create_texture(const unsigned char* pixels, int w, int h) {
    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glBindTexture(GL_TEXTURE_2D, 0);
    return (TextureID)tex;
}

void GLRenderingServer::set_viewport(int w, int h) {
    build_projection(w, h);
    glViewport(0, 0, w, h);
}

void GLRenderingServer::clear_screen(const Color& c) {
    glClearColor(c.r, c.g, c.b, c.a);
    glClear(GL_COLOR_BUFFER_BIT);
}

// Transform the quad's 4 corners into world space on the CPU, emit 6
// vertices. This is the heart of batching: no per-sprite uniform means
// no per-sprite draw call.
void GLRenderingServer::push_quad_vertices(const DrawItem& item) {
    const Transform2D& t = item.transform;
    Vector2 p0 = t.xform(Vector2(0, 0));
    Vector2 p1 = t.xform(Vector2(item.size.x, 0));
    Vector2 p2 = t.xform(Vector2(item.size.x, item.size.y));
    Vector2 p3 = t.xform(Vector2(0, item.size.y));

    float u0 = item.region.position.x;
    float v0 = item.region.position.y;
    float u1 = u0 + item.region.size.x;
    float v1 = v0 + item.region.size.y;

    const Color& c = item.modulate;
    auto V = [&](const Vector2& p, float u, float v) {
        m_vertices.push_back({p.x, p.y, u, v, c.r, c.g, c.b, c.a});
    };

    V(p0, u0, v0); V(p1, u1, v0); V(p2, u1, v1);
    V(p0, u0, v0); V(p2, u1, v1); V(p3, u0, v1);
}

void GLRenderingServer::emit_batch(TextureID texture) {
    if (m_vertices.empty()) return;

    m_shader->use();
    m_shader->setMat4("uProjection", m_projection);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, (GLuint)texture);
    GLint loc = glGetUniformLocation(m_shader->id(), "uTexture");
    glUniform1i(loc, 0);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

    // Grow the VBO only when needed; otherwise just refill it.
    size_t needed = m_vertices.size();
    if (needed > m_capacity) {
        glBufferData(GL_ARRAY_BUFFER, needed * sizeof(BatchVertex),
                     m_vertices.data(), GL_DYNAMIC_DRAW);
        m_capacity = needed;
    } else {
        glBufferSubData(GL_ARRAY_BUFFER, 0,
                        needed * sizeof(BatchVertex), m_vertices.data());
    }

    glDrawArrays(GL_TRIANGLES, 0, (GLsizei)needed);
    glBindVertexArray(0);
}

void GLRenderingServer::flush() {
    if (m_queue.empty()) return;

    // 1. Sort: z_index first (draw order), then texture (batch coherence).
    //    stable_sort keeps submission order within a z/texture group.
    std::stable_sort(m_queue.begin(), m_queue.end(),
        [](const DrawItem& a, const DrawItem& b) {
            if (a.z_index != b.z_index) return a.z_index < b.z_index;
            return a.texture < b.texture;
        });

    // 2. Accumulate vertices, flushing only when the texture changes.
    m_vertices.clear();
    TextureID current = m_queue.front().texture;
    for (const DrawItem& item : m_queue) {
        if (item.texture != current) {
            emit_batch(current);
            m_vertices.clear();
            current = item.texture;
        }
        push_quad_vertices(item);
    }
    emit_batch(current);

    m_vertices.clear();
    m_queue.clear();
}