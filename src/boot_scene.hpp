#pragma once

#include "scene.hpp"
#include "scene_stack.hpp"
#include "map.hpp"
#include "texture_manager.hpp"

class BootScene : public Scene {
public:
    BootScene(SceneStack& stack, TextureManager& textures);

    void handleEvent(const sf::Event& event) override;
    void update(float deltaTime) override;
    void draw(sf::RenderWindow& window) const override;

private:
    SceneStack& stack_;
    TextureManager& textures_;
    Map map_;
    bool loaded_ = false;
};

