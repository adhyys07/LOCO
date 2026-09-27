#pragma once
#include "scene/node.h"
#include "scene/sprite2d.h"
#include "servers/rendering_server.h"
#include <memory>

class SceneTree {
public:
    void set_root(std::unique_ptr<Node> root) {
        m_root = std::move(root);
        if (m_root) m_root->propagate_ready();
    }
    Node* get_root() const { return m_root.get(); }

    void set_paused(bool p) { m_paused = p; }
    bool is_paused() const { return m_paused; }

    void process(float dt) {
        if (m_root) m_root->propagate_process(dt);
    }

    void render(RenderingServer& rs) {
        if (m_root) draw_recursive(m_root.get(), rs);
        rs.flush();
    }

private:
    void draw_recursive(Node* node, RenderingServer& rs) {
        if (auto* sprite = dynamic_cast<Sprite2D*>(node)) sprite->draw(rs);
        for (auto& c : node->get_children()) draw_recursive(c.get(), rs);
    }
    std::unique_ptr<Node> m_root;
    bool m_paused = true;
};