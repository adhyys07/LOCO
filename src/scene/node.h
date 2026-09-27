#pragma once
#include <string>
#include <vector>
#include <memory>
class Node {
public:
    virtual ~Node() = default;
    virtual const char* get_class() const { return "Node"; }

    virtual void _ready() {}
    virtual void _process(float dt) {}

    Node* add_child(std::unique_ptr<Node> child) {
        child->m_parent = this;
        Node* raw = child.get();
        m_children.push_back(std::move(child));
        return raw;
    }

    // Detach and destroy a direct child. The editor needs this for
    // its delete button; games need it for despawning.
    bool remove_child(Node* child) {
        for (auto it = m_children.begin(); it != m_children.end(); ++it) {
            if (it->get() == child) { m_children.erase(it); return true; }
        }
        return false;
    }

    Node* get_parent() const { return m_parent; }
    const std::vector<std::unique_ptr<Node>>& get_children() const { return m_children; }

    Node* get_node(const std::string& name) const {
        for (auto& c : m_children) if (c->m_name == name) return c.get();
        return nullptr;
    }

    void set_name(const std::string& n) { m_name = n; }
    const std::string& get_name() const { return m_name; }

    void propagate_ready() {
        _ready();
        for (auto& c : m_children) c->propagate_ready();
    }
    void propagate_process(float dt) {
        _process(dt);
        for (auto& c : m_children) c->propagate_process(dt);
    }

protected:
    std::string m_name = "Node";
    Node* m_parent = nullptr;
    std::vector<std::unique_ptr<Node>> m_children;
};