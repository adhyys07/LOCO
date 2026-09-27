#pragma once
#include "scene/scene_tree.h"
#include <string>
#include <vector>

struct SDL_Window;
union SDL_Event;
class Framebuffer;

class Editor {
public:
    bool init(SDL_Window* window, void* glContext);
    void shutdown();

    void process_event(const SDL_Event& e);   
    bool viewport_hovered() const { return m_viewportHovered; }
    void draw(SceneTree& tree, Framebuffer& fb);               
    void render();                            

    Node* get_selected() const { return m_selected; }
    void log(const std::string& msg);
    int viewport_width() const  { return m_viewportW; }
    int viewport_height() const { return m_viewportH; }

private:
    void draw_menu_bar(SceneTree& tree);
    void draw_dockspace();
    void draw_toolbar(SceneTree& tree);
    void draw_scene_panel(SceneTree& tree);
    void draw_tree_node(Node* node);
    void draw_inspector_panel();
    void draw_viewport_panel(Framebuffer& fb);
    void draw_output_panel();
    void draw_add_node_dialog();

    Node* create_node_by_type(const char* type);

    Node* m_selected = nullptr;
    Node* m_pendingDelete = nullptr;
    Node* m_addChildTarget = nullptr;
    bool  m_openAddDialog = false;
    int   m_addTypeIndex = 0;

    int  m_viewportW = 1, m_viewportH = 1;
    bool m_viewportHovered = false;
    bool m_initialized = false;
    bool m_firstLayout = true;
    bool m_showDemo = false;

    std::vector<std::string> m_log;
};
