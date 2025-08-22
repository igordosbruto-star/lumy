#pragma once

#include <SFML/Graphics.hpp>

class Scene {
public:
    explicit Scene(const sf::Vector2f& startPos);

    void handleEvent(const sf::Event& event);
    void update(float deltaTime);
    void draw(sf::RenderWindow& window) const;

private:
    sf::RectangleShape hero;
    float moveSpeed = 200.f;
};

