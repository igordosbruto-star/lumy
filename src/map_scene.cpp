#include "map_scene.hpp"
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Event.hpp>
#include <tmxlite/Map.hpp>

MapScene::MapScene(TextureManager& textures, const std::string& tmxPath)
    : textures_(textures), map_(textures_) {
    map_.load(tmxPath);

    sf::Vector2f startPos{0.f, 0.f};
    tmx::Map tmxMap;
    if (tmxMap.load(tmxPath)) {
        for (const auto& prop : tmxMap.getProperties()) {
            if (prop.getName() == "spawn_x") {
                startPos.x = static_cast<float>(prop.getIntValue());
            } else if (prop.getName() == "spawn_y") {
                startPos.y = static_cast<float>(prop.getIntValue());
            }
        }
    }

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
    // Draw ground_* layers first
    for (std::size_t i = 0; i < map_.getLayerCount(); ++i) {
        const std::string& name = map_.getLayerName(i);
        if (name.rfind("ground_", 0) == 0) {
            map_.drawLayer(i, window);
        }
    }

    window.draw(hero_);

    // Draw object_* and remaining layers
    for (std::size_t i = 0; i < map_.getLayerCount(); ++i) {
        const std::string& name = map_.getLayerName(i);
        if (name.rfind("ground_", 0) != 0) {
            map_.drawLayer(i, window);
        }
    }
}
