#pragma once

#include "scene.hpp"
#include "scene_stack.hpp"
#include <SFML/Graphics.hpp>

class TitleScene : public Scene {
public:
    explicit TitleScene(SceneStack& stack);

    void handleEvent(const sf::Event& event) override;
    void update(float deltaTime) override;
    void draw(sf::RenderWindow& window) const override;

private:
    SceneStack& stack_;
    sf::Font font_;
    sf::Text startText_;
};

