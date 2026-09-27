#pragma once
#include "math2d.h"
#include <vector>
#include <cstdint>

using TextureID = uint32_t;

struct DrawItem {
    Transform2D transform;
    Vector2 size{64, 64};
    Color modulate;
    TextureID texture = 0;              // 0 = flat color (white 1x1 texture)
    Rect2 region{{0, 0}, {1, 1}};       // UV sub-rect, for sprite sheets
    int z_index = 0;
};

// Nodes submit draw items; the backend batches them.
class RenderingServer {
public:
    virtual ~RenderingServer() = default;

    void submit(const DrawItem& item) { m_queue.push_back(item); }

    virtual void clear_screen(const Color& c) = 0;
    virtual void flush() = 0;
    virtual TextureID white_texture() const = 0;
    virtual void set_viewport(int w, int h) = 0;

protected:
    std::vector<DrawItem> m_queue;
};