#include "engine.h"
#include "gl_rendering_server.h"
#include "framebuffer.h"
#include "input.h"
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <iostream>

Engine::Engine(const std::string& title, int width, int height)
    : m_title(title), m_width(width), m_height(height) {}

Engine::~Engine() {
    m_editor.shutdown();
    m_framebuffer.reset();
    m_server.reset();
    if (m_glContext) SDL_GL_DeleteContext((SDL_GLContext)m_glContext);
    if (m_window) SDL_DestroyWindow(m_window);
    SDL_Quit();
}

bool Engine::init(bool enable_editor) {
    m_editorEnabled = enable_editor;
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
        return false;
    }
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    m_window = SDL_CreateWindow(m_title.c_str(),
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        m_width, m_height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE |
                           SDL_WINDOW_MAXIMIZED);
    if (!m_window) {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << std::endl;
        return false;
    }
    m_glContext = SDL_GL_CreateContext(m_window);
    if (!m_glContext) {
        std::cerr << "SDL_GL_CreateContext failed: " << SDL_GetError() << std::endl;
        return false;
    }
    SDL_GL_SetSwapInterval(1);

    glewExperimental = GL_TRUE;
    GLenum status = glewInit();
    if (status != GLEW_OK) {
        std::cerr << "glewInit failed: " << glewGetErrorString(status) << std::endl;
        return false;
    }

    m_server = std::make_unique<GLRenderingServer>(m_width, m_height);
    m_framebuffer = std::make_unique<Framebuffer>(m_width, m_height);

    if (m_editorEnabled && !m_editor.init(m_window, m_glContext)) {
        std::cerr << "Editor init failed; running without it." << std::endl;
        m_editorEnabled = false;
    }
    if (!m_editorEnabled) m_tree.set_paused(false);
    return true;
}

void Engine::run() {
    m_running = true;
    const double FIXED_DT = 1.0 / 60.0;
    double accumulator = 0.0;
    Uint64 prevTicks = SDL_GetPerformanceCounter();
    const double perfFreq = (double)SDL_GetPerformanceFrequency();

    while (m_running) {
        Uint64 nowTicks = SDL_GetPerformanceCounter();
        double frameTime = (nowTicks - prevTicks) / perfFreq;
        prevTicks = nowTicks;
        if (frameTime > 0.25) frameTime = 0.25;
        accumulator += frameTime;

        Input::beginFrame();
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (m_editorEnabled) m_editor.process_event(event);
            if (event.type == SDL_QUIT) m_running = false;
            if (event.type == SDL_WINDOWEVENT &&
                event.window.event == SDL_WINDOWEVENT_RESIZED) {
                m_width = event.window.data1;
                m_height = event.window.data2;
            }
            Input::handleEvent(event);
        }
        if (Input::isPressed(SDLK_ESCAPE)) m_running = false;

        if (m_editorEnabled) m_editor.draw(m_tree, *m_framebuffer);

        while (accumulator >= FIXED_DT) {
            m_tree.process((float)FIXED_DT);
            accumulator -= FIXED_DT;
        }

        if (m_editorEnabled) {
            // Game renders into the framebuffer, sized to the Viewport panel.
            int vw = m_editor.viewport_width();
            int vh = m_editor.viewport_height();
            m_framebuffer->resize(vw, vh);
            m_server->set_viewport(vw, vh);
            m_framebuffer->bind();
            m_server->clear_screen(Color(0.11f, 0.11f, 0.13f, 1.0f));
            m_tree.render(*m_server);
            m_framebuffer->unbind();

            glViewport(0, 0, m_width, m_height);
            glClearColor(0.06f, 0.06f, 0.07f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            m_editor.render();
        } else {
            m_server->set_viewport(m_width, m_height);
            glViewport(0, 0, m_width, m_height);
            m_server->clear_screen(Color(0.11f, 0.11f, 0.13f, 1.0f));
            m_tree.render(*m_server);
        }

        SDL_GL_SwapWindow(m_window);
    }
}