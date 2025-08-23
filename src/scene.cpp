#include "scene.hpp"
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Event.hpp>

Scene::Scene(const sf::Vector2f& startPos) {
    hero.setSize(sf::Vector2f{64.f, 64.f});
    hero.setFillColor(sf::Color::White);
    hero.setOrigin(sf::Vector2f{32.f, 32.f});
    hero.setPosition(startPos);
}

void Scene::handleEvent(const sf::Event& event) {
    if (const auto* mouse = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mouse->button == sf::Mouse::Button::Left) {
            hero.setPosition({static_cast<float>(mouse->position.x),
                              static_cast<float>(mouse->position.y)});
        }
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

