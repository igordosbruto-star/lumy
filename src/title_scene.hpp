#pragma once

#include "scene.hpp"
#include "scene_stack.hpp"
#include "texture_manager.hpp"
#include <SFML/Graphics.hpp>

class TitleScene : public Scene {
public:
    explicit TitleScene(SceneStack& stack, TextureManager& textures);

    void handleEvent(const sf::Event& event) override;
    void update(float deltaTime) override;
    void draw(sf::RenderWindow& window) const override;

private:
    SceneStack& stack_;
    TextureManager& textures_;
    sf::Font font_;
    sf::Text startText_;
};

