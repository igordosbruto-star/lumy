#include "scene.hpp"
#include <SFML/Window/Keyboard.hpp>

Scene::Scene(const sf::Vector2f& startPos) {
    hero.setSize(sf::Vector2f{64.f, 64.f});
    hero.setFillColor(sf::Color::White);
    hero.setOrigin(sf::Vector2f{32.f, 32.f});
    hero.setPosition(startPos);
}

// FIX THIS
void Scene::handleEvent(const sf::Event& event) {
    if (event.type == sf::Event::MouseButtonPressed &&
        event.mouseButton.button == sf::Mouse::Left) {
        hero.setPosition(
            sf::Vector2f{static_cast<float>(event.mouseButton.x),
                          static_cast<float>(event.mouseButton.y)});
    }
}

void Scene::update(float deltaTime) {
    sf::Vector2f pos = hero.getPosition();
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
        pos.y -= moveSpeed * deltaTime;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
        pos.y += moveSpeed * deltaTime;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
        pos.x -= moveSpeed * deltaTime;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
        pos.x += moveSpeed * deltaTime;
    hero.setPosition(pos);
}

void Scene::draw(sf::RenderWindow& window) const {
    window.draw(hero);
}

