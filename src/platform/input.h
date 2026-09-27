#pragma once
#include <SDL2/SDL.h>
#include <unordered_map>

// Central keyboard state. The Engine feeds it SDL events each frame.
// Query it from anywhere in gameplay code without touching SDL directly.
class Input {
public:
    static void beginFrame();                 // clear per-frame "pressed" state
    static void handleEvent(const SDL_Event& e);

    static bool isDown(SDL_Keycode key);      // true while held
    static bool isPressed(SDL_Keycode key);   // true only on the frame pressed

private:
    static std::unordered_map<SDL_Keycode, bool> s_down;
    static std::unordered_map<SDL_Keycode, bool> s_pressed;
};
