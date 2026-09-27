#pragma once
#include <memory>
#include <string>
#include "scene_tree.h"
#include "editor.h"

struct SDL_Window;
class GLRenderingServer;
class Framebuffer;

class Engine {
public:
    Engine(const std::string& title, int width, int height);
    ~Engine();
    bool init(bool enable_editor = true);
    void run();
    SceneTree& get_tree() { return m_tree; }
    Editor& get_editor() { return m_editor; }
    GLRenderingServer* get_rendering_server() { return m_server.get(); }
private:
    std::string m_title;
    int m_width, m_height;
    SDL_Window* m_window = nullptr;
    void* m_glContext = nullptr;
    std::unique_ptr<GLRenderingServer> m_server;
    std::unique_ptr<Framebuffer> m_framebuffer;
    SceneTree m_tree;
    Editor m_editor;
    bool m_editorEnabled = true;
    bool m_running = false;
};