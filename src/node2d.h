#pragma once
#include "node.h"
#include "math2d.h"

class Node2D : public Node {
public:
    const char* get_class() const override { return "Node2D"; }

    void set_position(const Vector2& p) { m_position = p; }
    void set_rotation(float r)          { m_rotation = r; }
    void set_scale(const Vector2& s)    { m_scale = s; }

    const Vector2& get_position() const { return m_position; }
    float get_rotation() const { return m_rotation; }
    const Vector2& get_scale() const { return m_scale; }

    // Non-const refs so the editor can bind widgets straight to them.
    Vector2& position_ref() { return m_position; }
    float&   rotation_ref() { return m_rotation; }
    Vector2& scale_ref()    { return m_scale; }

    Transform2D get_transform() const {
        return Transform2D::from_trs(m_position, m_rotation, m_scale);
    }
    Transform2D get_global_transform() const {
        Node2D* p = dynamic_cast<Node2D*>(m_parent);
        return p ? p->get_global_transform() * get_transform() : get_transform();
    }

    void set_z_index(int z) { m_z_index = z; }
    int get_z_index() const { return m_z_index; }
    int& z_index_ref() { return m_z_index; }
    void set_visible(bool v) { m_visible = v; }
    bool is_visible() const { return m_visible; }
    bool& visible_ref() { return m_visible; }

protected:
    Vector2 m_position{0, 0};
    float   m_rotation = 0.0f;
    Vector2 m_scale{1, 1};
    int     m_z_index = 0;
    bool    m_visible = true;
};