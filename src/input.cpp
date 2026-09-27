#include "input.h"

std::unordered_map<SDL_Keycode, bool> Input::s_down;
std::unordered_map<SDL_Keycode, bool> Input::s_pressed;

void Input::beginFrame() {
    s_pressed.clear();
}

void Input::handleEvent(const SDL_Event& e) {
    if (e.type == SDL_KEYDOWN && e.key.repeat == 0) {
        s_down[e.key.keysym.sym] = true;
        s_pressed[e.key.keysym.sym] = true;
    } else if (e.type == SDL_KEYUP) {
        s_down[e.key.keysym.sym] = false;
    }
}

bool Input::isDown(SDL_Keycode key) {
    auto it = s_down.find(key);
    return it != s_down.end() && it->second;
}

bool Input::isPressed(SDL_Keycode key) {
    auto it = s_pressed.find(key);
    return it != s_pressed.end() && it->second;
}