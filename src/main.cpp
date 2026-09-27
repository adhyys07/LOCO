#include "engine.h"
#include "sprite2d.h"
#include "input.h"
#include <SDL2/SDL.h>
#include <memory>

class Player : public Node2D {
public:
    const char* get_class() const override { return "Player"; }

    void _ready() override {
        auto sprite = std::make_unique<Sprite2D>();
        sprite->set_name("Sprite");
        sprite->set_size({50, 50});
        sprite->set_modulate(Color(0.9f, 0.4f, 0.2f));
        add_child(std::move(sprite));
    }
    void _process(float dt) override {
        Vector2 dir;
        if (Input::isDown(SDLK_a) || Input::isDown(SDLK_LEFT))  dir.x -= 1;
        if (Input::isDown(SDLK_d) || Input::isDown(SDLK_RIGHT)) dir.x += 1;
        if (Input::isDown(SDLK_w) || Input::isDown(SDLK_UP))    dir.y -= 1;
        if (Input::isDown(SDLK_s) || Input::isDown(SDLK_DOWN))  dir.y += 1;
        set_position(get_position() + dir.normalized() * (m_speed * dt));
    }
private:
    float m_speed = 220.0f;
};

int main(int argc, char** argv) {
    Engine engine("LOCO Editor", 1280, 720);
    if (!engine.init(true)) return 1;

    auto root = std::make_unique<Node2D>();
    root->set_name("Root");

    auto player = std::make_unique<Player>();
    player->set_name("Player");
    player->set_position({500, 300});

    auto orbit = std::make_unique<Sprite2D>();
    orbit->set_name("Orbiter");
    orbit->set_position({90, 0});
    orbit->set_size({24, 24});
    orbit->set_modulate(Color(0.4f, 0.8f, 0.4f));
    player->add_child(std::move(orbit));

    root->add_child(std::move(player));
    engine.get_tree().set_root(std::move(root));
    engine.run();
    return 0;
}