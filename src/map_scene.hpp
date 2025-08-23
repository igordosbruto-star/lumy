#pragma once

#include "scene.hpp"
#include <SFML/Graphics.hpp>

class MapScene : public Scene {
public:
    explicit MapScene(const sf::Vector2f& startPos);

    void handleEvent(const sf::Event& event) override;
    void update(float deltaTime) override;
    void draw(sf::RenderWindow& window) const override;

private:
    sf::RectangleShape hero_;
    float moveSpeed_ = 200.f;
};
