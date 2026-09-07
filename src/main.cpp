#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <iostream>
#include "renderer.h"

int main(int argc, char** argv) {
    const int WINDOW_W = 800;
    const int WINDOW_H = 600;

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    SDL_Window* window = SDL_CreateWindow(
        "Engine2D",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_W, WINDOW_H,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
    );
    if (!window) {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_GLContext glContext = SDL_GL_CreateContext(window);
    if (!glContext) {
        std::cerr << "SDL_GL_CreateContext failed: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    SDL_GL_SetSwapInterval(1);

    glewExperimental = GL_TRUE;
    GLenum glewStatus = glewInit();
    if (glewStatus != GLEW_OK) {
        std::cerr << "glewInit failed: " << glewGetErrorString(glewStatus) << std::endl;
        return 1;
    }

    Renderer2D renderer(WINDOW_W, WINDOW_H);

    float quadX = 350.0f, quadY = 250.0f;
    const float speed = 200.0f;

    const double FIXED_DT = 1.0 / 60.0;
    double accumulator = 0.0;
    Uint64 prevTicks = SDL_GetPerformanceCounter();
    const double perfFreq = (double)SDL_GetPerformanceFrequency();

    bool running = true;
    bool left = false, right = false, up = false, down = false;

    while (running) {
        Uint64 nowTicks = SDL_GetPerformanceCounter();
        double frameTime = (nowTicks - prevTicks) / perfFreq;
        prevTicks = nowTicks;
        if (frameTime > 0.25) frameTime = 0.25;
        accumulator += frameTime;

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED) {
                renderer.resize(event.window.data1, event.window.data2);
            }
            if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
                bool down_ = (event.type == SDL_KEYDOWN);
                switch (event.key.keysym.sym) {
                    case SDLK_LEFT:  case SDLK_a: left = down_; break;
                    case SDLK_RIGHT: case SDLK_d: right = down_; break;
                    case SDLK_UP:    case SDLK_w: up = down_; break;
                    case SDLK_DOWN:  case SDLK_s: down = down_; break;
                    case SDLK_ESCAPE: running = false; break;
                }
            }
        }

        while (accumulator >= FIXED_DT) {
            if (left)  quadX -= speed * FIXED_DT;
            if (right) quadX += speed * FIXED_DT;
            if (up)    quadY -= speed * FIXED_DT;
            if (down)  quadY += speed * FIXED_DT;
            accumulator -= FIXED_DT;
        }

        renderer.beginFrame(0.1f, 0.1f, 0.12f, 1.0f);
        renderer.drawQuad(quadX, quadY, 50.0f, 50.0f, 0.9f, 0.4f, 0.2f, 1.0f);
        SDL_GL_SwapWindow(window);
    }

    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}