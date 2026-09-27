#pragma once
#include "node2d.h"
#include "rendering_server.h"

class Sprite2D : public Node2D {
public:
    const char* get_class() const override { return "Sprite2D"; }

    void set_texture(TextureID t) { m_texture = t; }
    void set_size(const Vector2& s) { m_size = s; }
    void set_modulate(const Color& c) { m_modulate = c; }
    void set_region(const Rect2& r) { m_region = r; }
    TextureID get_texture() const { return m_texture; }

    Vector2& size_ref() { return m_size; }
    Color&   modulate_ref() { return m_modulate; }

    void draw(RenderingServer& rs) {
        if (!m_visible) return;
        DrawItem item;
        item.transform = get_global_transform();
        item.size      = m_size;
        item.modulate  = m_modulate;
        item.texture   = m_texture ? m_texture : rs.white_texture();
        item.region    = m_region;
        item.z_index   = m_z_index;
        rs.submit(item);
    }
private:
    TextureID m_texture = 0;
    Vector2 m_size{64, 64};
    Color m_modulate;
    Rect2 m_region{{0, 0}, {1, 1}};
};
