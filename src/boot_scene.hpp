#pragma once

#include "scene.hpp"
#include "scene_stack.hpp"

class BootScene : public Scene {
public:
    explicit BootScene(SceneStack& stack);

    void handleEvent(const sf::Event& event) override;
    void update(float deltaTime) override;
    void draw(sf::RenderWindow& window) const override;

private:
    SceneStack& stack_;
    bool loaded_ = false;
};

