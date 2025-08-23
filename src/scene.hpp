#pragma once

#include <SFML/Graphics.hpp>

class Scene {
public:
    explicit Scene(const sf::Vector2f& startPos);
    virtual ~Scene() = default;

    virtual void handleEvent(const sf::Event& event);
    virtual void update(float deltaTime);
    virtual void draw(sf::RenderWindow& window) const;

private:
    sf::RectangleShape hero;
    float moveSpeed = 200.f;
};

