#include "editor.h"
#include "node2d.h"
#include "sprite2d.h"
#include "framebuffer.h"

#include "imgui.h"
#include "imgui_internal.h"          // for the dock-builder API
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_opengl3.h"

#include <SDL2/SDL.h>
#include <cstdio>
#include <cstring>

static const char* kNodeTypes[] = { "Node2D", "Sprite2D" };

bool Editor::init(SDL_Window* window, void* glContext) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.IniFilename = "loco_editor_layout.ini";   // remembers your layout

    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 4.0f;
    style.FrameRounding  = 3.0f;
    style.TabRounding    = 3.0f;
    style.WindowPadding  = ImVec2(8, 8);

    if (!ImGui_ImplSDL2_InitForOpenGL(window, glContext)) return false;
    if (!ImGui_ImplOpenGL3_Init("#version 330")) return false;

    m_initialized = true;
    log("LOCO editor ready.");
    return true;
}

void Editor::shutdown() {
    if (!m_initialized) return;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    m_initialized = false;
}

void Editor::process_event(const SDL_Event& e) {
    if (m_initialized) ImGui_ImplSDL2_ProcessEvent(&e);
}

void Editor::log(const std::string& msg) {
    m_log.push_back(msg);
    if (m_log.size() > 500) m_log.erase(m_log.begin());
}

Node* Editor::create_node_by_type(const char* type) {
    if (std::strcmp(type, "Sprite2D") == 0) {
        auto s = std::make_unique<Sprite2D>();
        s->set_name("Sprite2D");
        s->set_size({64, 64});
        return s.release();
    }
    auto n = std::make_unique<Node2D>();
    n->set_name("Node2D");
    return n.release();
}

void Editor::draw_dockspace() {
    ImGuiViewport* vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(vp->WorkPos);
    ImGui::SetNextWindowSize(vp->WorkSize);
    ImGui::SetNextWindowViewport(vp->ID);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDocking |
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("##DockHost", nullptr, flags);
    ImGui::PopStyleVar(3);

    ImGuiID dockId = ImGui::GetID("LocoDockspace");
    ImGui::DockSpace(dockId, ImVec2(0, 0), ImGuiDockNodeFlags_None);

    // Build the default layout once; after that the .ini file wins.
    if (m_firstLayout) {
        m_firstLayout = false;
        if (ImGui::DockBuilderGetNode(dockId) == nullptr ||
            ImGui::DockBuilderGetNode(dockId)->IsEmpty()) {
            ImGui::DockBuilderRemoveNode(dockId);
            ImGui::DockBuilderAddNode(dockId, ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderSetNodeSize(dockId, vp->WorkSize);

            ImGuiID center = dockId;
            ImGuiID left   = ImGui::DockBuilderSplitNode(center, ImGuiDir_Left,  0.20f, nullptr, &center);
            ImGuiID right  = ImGui::DockBuilderSplitNode(center, ImGuiDir_Right, 0.25f, nullptr, &center);
            ImGuiID bottom = ImGui::DockBuilderSplitNode(center, ImGuiDir_Down,  0.25f, nullptr, &center);
            ImGuiID top    = ImGui::DockBuilderSplitNode(center, ImGuiDir_Up,    0.08f, nullptr, &center);

            ImGui::DockBuilderDockWindow("Scene",     left);
            ImGui::DockBuilderDockWindow("Inspector", right);
            ImGui::DockBuilderDockWindow("Output",    bottom);
            ImGui::DockBuilderDockWindow("Toolbar",   top);
            ImGui::DockBuilderDockWindow("Viewport",  center);
            ImGui::DockBuilderFinish(dockId);
        }
    }
    ImGui::End();
}

void Editor::draw_menu_bar(SceneTree& tree) {
    if (!ImGui::BeginMainMenuBar()) return;

    if (ImGui::BeginMenu("Scene")) {
        if (ImGui::MenuItem("New Root Node2D")) {
            auto root = std::make_unique<Node2D>();
            root->set_name("Root");
            tree.set_root(std::move(root));
            m_selected = tree.get_root();
            log("New scene created.");
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Save Scene", "Ctrl+S", false, false)) {}
        if (ImGui::MenuItem("Open Scene", "Ctrl+O", false, false)) {}
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Node")) {
        bool has = m_selected != nullptr;
        if (ImGui::MenuItem("Add Child Node...", "Ctrl+A", false, has)) {
            m_addChildTarget = m_selected;
            m_openAddDialog = true;
        }
        if (ImGui::MenuItem("Delete Node", "Del", false,
                            has && m_selected->get_parent())) {
            m_pendingDelete = m_selected;
        }
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("View")) {
        ImGui::MenuItem("ImGui Demo", nullptr, &m_showDemo);
        if (ImGui::MenuItem("Reset Layout")) {
            m_firstLayout = true;
            log("Layout reset.");
        }
        ImGui::EndMenu();
    }

    char fps[64];
    std::snprintf(fps, sizeof(fps), "%.1f FPS", ImGui::GetIO().Framerate);
    float w = ImGui::CalcTextSize(fps).x;
    ImGui::SameLine(ImGui::GetWindowWidth() - w - 20.0f);
    ImGui::TextUnformatted(fps);

    ImGui::EndMainMenuBar();
}

void Editor::draw_toolbar(SceneTree& tree) {
    ImGui::Begin("Toolbar");
    bool paused = tree.is_paused();
    if (ImGui::Button(paused ? "> Play" : "|| Pause", ImVec2(90, 0))) {
        tree.set_paused(!paused);
        log(paused ? "Playing." : "Paused.");
    }
    ImGui::SameLine();
    if (ImGui::Button("Stop", ImVec2(70, 0))) {
        tree.set_paused(true);
        log("Stopped.");
    }
    ImGui::SameLine();
    ImGui::TextDisabled("|");
    ImGui::SameLine();
    ImGui::Text("%s", paused ? "Editing" : "Running");
    ImGui::End();
}

void Editor::draw_tree_node(Node* node) {
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow |
                               ImGuiTreeNodeFlags_SpanAvailWidth |
                               ImGuiTreeNodeFlags_DefaultOpen;
    if (node->get_children().empty()) flags |= ImGuiTreeNodeFlags_Leaf;
    if (node == m_selected)           flags |= ImGuiTreeNodeFlags_Selected;

    ImGui::PushID((void*)node);
    bool open = ImGui::TreeNodeEx((void*)node, flags, "%s", node->get_name().c_str());

    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) m_selected = node;

    ImGui::SameLine();
    ImGui::TextDisabled("(%s)", node->get_class());

    if (ImGui::BeginPopupContextItem("node_ctx")) {
        m_selected = node;
        if (ImGui::MenuItem("Add Child Node...")) {
            m_addChildTarget = node;
            m_openAddDialog = true;
        }
        if (ImGui::MenuItem("Duplicate", nullptr, false, node->get_parent() != nullptr)) {
            if (auto* src = dynamic_cast<Node2D*>(node)) {
                Node* copy = create_node_by_type(src->get_class());
                if (auto* dst = dynamic_cast<Node2D*>(copy)) {
                    dst->set_position(src->get_position() + Vector2(20, 20));
                    dst->set_rotation(src->get_rotation());
                    dst->set_scale(src->get_scale());
                    dst->set_name(src->get_name() + "_copy");
                }
                if (auto* ss = dynamic_cast<Sprite2D*>(src)) {
                    if (auto* ds = dynamic_cast<Sprite2D*>(copy)) {
                        ds->set_size(ss->size_ref());
                        ds->set_modulate(ss->modulate_ref());
                    }
                }
                m_selected = node->get_parent()->add_child(
                    std::unique_ptr<Node>(copy));
                log("Duplicated node.");
            }
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Delete", nullptr, false, node->get_parent() != nullptr)) {
            m_pendingDelete = node;
        }
        ImGui::EndPopup();
    }

    if (open) {
        for (auto& child : node->get_children()) draw_tree_node(child.get());
        ImGui::TreePop();
    }
    ImGui::PopID();
}

void Editor::draw_scene_panel(SceneTree& tree) {
    ImGui::Begin("Scene");

    if (ImGui::Button("+ Add Node", ImVec2(-1, 0))) {
        m_addChildTarget = m_selected ? m_selected : tree.get_root();
        m_openAddDialog = true;
    }
    ImGui::Separator();

    if (Node* root = tree.get_root()) {
        draw_tree_node(root);
    } else {
        ImGui::TextDisabled("No scene. Scene > New Root Node2D");
    }
    ImGui::End();

    // Deferred so we never free a node mid-traversal.
    if (m_pendingDelete) {
        Node* parent = m_pendingDelete->get_parent();
        if (parent) {
            if (m_selected == m_pendingDelete) m_selected = parent;
            log("Deleted " + m_pendingDelete->get_name());
            parent->remove_child(m_pendingDelete);
        }
        m_pendingDelete = nullptr;
    }
}

void Editor::draw_add_node_dialog() {
    if (m_openAddDialog) {
        ImGui::OpenPopup("Create New Node");
        m_openAddDialog = false;
    }

    ImGui::SetNextWindowSize(ImVec2(320, 0), ImGuiCond_Appearing);
    if (!ImGui::BeginPopupModal("Create New Node", nullptr,
                                ImGuiWindowFlags_AlwaysAutoResize)) return;

    ImGui::TextDisabled("Parent: %s",
        m_addChildTarget ? m_addChildTarget->get_name().c_str() : "(none)");
    ImGui::Separator();

    for (int i = 0; i < IM_ARRAYSIZE(kNodeTypes); ++i) {
        if (ImGui::Selectable(kNodeTypes[i], m_addTypeIndex == i))
            m_addTypeIndex = i;
    }

    ImGui::Separator();
    if (ImGui::Button("Create", ImVec2(140, 0)) && m_addChildTarget) {
        Node* n = create_node_by_type(kNodeTypes[m_addTypeIndex]);
        m_selected = m_addChildTarget->add_child(std::unique_ptr<Node>(n));
        log(std::string("Added ") + kNodeTypes[m_addTypeIndex]);
        ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel", ImVec2(140, 0))) ImGui::CloseCurrentPopup();

    ImGui::EndPopup();
}


void Editor::draw_inspector_panel() {
    ImGui::Begin("Inspector");

    if (!m_selected) {
        ImGui::TextDisabled("Select a node in the Scene panel.");
        ImGui::End();
        return;
    }

    ImGui::TextColored(ImVec4(0.5f, 0.8f, 1.0f, 1.0f), "%s", m_selected->get_class());
    ImGui::Separator();

    char nameBuf[128];
    std::snprintf(nameBuf, sizeof(nameBuf), "%s", m_selected->get_name().c_str());
    if (ImGui::InputText("Name", nameBuf, sizeof(nameBuf)))
        m_selected->set_name(nameBuf);

    if (auto* n2d = dynamic_cast<Node2D*>(m_selected)) {
        if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::DragFloat2("Position", &n2d->position_ref().x, 1.0f);
            float deg = n2d->get_rotation() * 57.2957795f;
            if (ImGui::DragFloat("Rotation", &deg, 0.5f))
                n2d->set_rotation(deg / 57.2957795f);
            ImGui::DragFloat2("Scale", &n2d->scale_ref().x, 0.01f, 0.01f, 100.0f);
        }
        if (ImGui::CollapsingHeader("Visibility", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Checkbox("Visible", &n2d->visible_ref());
            ImGui::DragInt("Z Index", &n2d->z_index_ref(), 1, -100, 100);
        }
    }

    if (auto* sprite = dynamic_cast<Sprite2D*>(m_selected)) {
        if (ImGui::CollapsingHeader("Sprite2D", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::DragFloat2("Size", &sprite->size_ref().x, 1.0f, 1.0f, 4096.0f);
            ImGui::ColorEdit4("Modulate", &sprite->modulate_ref().r);
        }
    }

    ImGui::End();
}

void Editor::draw_viewport_panel(Framebuffer& fb) {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("Viewport");

    ImVec2 avail = ImGui::GetContentRegionAvail();
    m_viewportW = (int)avail.x > 0 ? (int)avail.x : 1;
    m_viewportH = (int)avail.y > 0 ? (int)avail.y : 1;
    m_viewportHovered = ImGui::IsWindowHovered();

    ImGui::Image((ImTextureID)(intptr_t)fb.color_texture(),
                 ImVec2((float)m_viewportW, (float)m_viewportH),
                 ImVec2(0, 1), ImVec2(1, 0));

    ImGui::End();
    ImGui::PopStyleVar();
}

void Editor::draw_output_panel() {
    ImGui::Begin("Output");
    if (ImGui::SmallButton("Clear")) m_log.clear();
    ImGui::Separator();
    ImGui::BeginChild("log_scroll");
    for (const std::string& line : m_log)
        ImGui::TextUnformatted(line.c_str());
    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 1.0f)
        ImGui::SetScrollHereY(1.0f);
    ImGui::EndChild();
    ImGui::End();
}

void Editor::draw(SceneTree& tree, Framebuffer& fb) {
    if (!m_initialized) return;

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    draw_menu_bar(tree);
    draw_dockspace();
    draw_toolbar(tree);
    draw_scene_panel(tree);
    draw_inspector_panel();
    draw_viewport_panel(fb);
    draw_output_panel();
    draw_add_node_dialog();

    if (m_showDemo) ImGui::ShowDemoWindow(&m_showDemo);

    ImGui::Render();
}

void Editor::render() {
    if (!m_initialized) return;
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}