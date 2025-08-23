#include "map_scene.hpp"
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Event.hpp>

MapScene::MapScene(const sf::Vector2f& startPos) {
    hero_.setSize(sf::Vector2f{64.f, 64.f});
    hero_.setFillColor(sf::Color::White);
    hero_.setOrigin(sf::Vector2f{32.f, 32.f});
    hero_.setPosition(startPos);
}

void MapScene::handleEvent(const sf::Event& event) {
    if (const auto* mouse = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mouse->button == sf::Mouse::Button::Left) {
            hero_.setPosition({static_cast<float>(mouse->position.x),
                               static_cast<float>(mouse->position.y)});
        }
    }
}

void MapScene::update(float deltaTime) {
    sf::Vector2f pos = hero_.getPosition();
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
        pos.y -= moveSpeed_ * deltaTime;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
        pos.y += moveSpeed_ * deltaTime;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
        pos.x -= moveSpeed_ * deltaTime;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
        pos.x += moveSpeed_ * deltaTime;
    hero_.setPosition(pos);
}

void MapScene::draw(sf::RenderWindow& window) const {
    window.draw(hero_);
}
