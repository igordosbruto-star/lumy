#pragma once

#include <SFML/Graphics.hpp>

class Scene {
public:
    Scene() = default;
    virtual ~Scene();

    virtual void handleEvent(const sf::Event& event) = 0;
    virtual void update(float deltaTime) = 0;
    virtual void draw(sf::RenderWindow& window) const = 0;
};

