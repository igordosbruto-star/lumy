#include "map_scene.hpp"
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Event.hpp>
#include <tmxlite/Map.hpp>
#include <tmxlite/Layer.hpp>
#include <tmxlite/Object.hpp>
#include <tmxlite/ObjectGroup.hpp>

MapScene::MapScene(TextureManager& textures, const std::string& tmxPath)
    : textures_(textures), map_(textures_) {
    map_.load(tmxPath);

    sf::Vector2f startPos{0.f, 0.f};
    tmx::Map tmxMap;
    if (tmxMap.load(tmxPath)) {
        bool found = false;
        for (const auto& layer : tmxMap.getLayers()) {
            if (layer->getType() != tmx::Layer::Type::Object)
                continue;
            const auto& group = layer->getLayerAs<tmx::ObjectGroup>();
            for (const auto& obj : group.getObjects()) {
                const auto& name = obj.getName();
                if (name == "player" || name == "spawn") {
                    const auto pos = obj.getPosition();
                    startPos.x = pos.x;
                    startPos.y = pos.y;
                    found = true;
                    break;
                }
            }
            if (found)
                break;
        }
        if (!found) {
            for (const auto& prop : tmxMap.getProperties()) {
                if (prop.getName() == "spawn_x") {
                    startPos.x = static_cast<float>(prop.getIntValue());
                } else if (prop.getName() == "spawn_y") {
                    startPos.y = static_cast<float>(prop.getIntValue());
                }
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
    sf::Vector2f newPos = pos;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
        newPos.y -= moveSpeed_ * deltaTime;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
        newPos.y += moveSpeed_ * deltaTime;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
        newPos.x -= moveSpeed_ * deltaTime;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
        newPos.x += moveSpeed_ * deltaTime;

    const auto &ts = map_.getTileSize();
    unsigned tileX = static_cast<unsigned>(newPos.x / static_cast<float>(ts.x));
    unsigned tileY = static_cast<unsigned>(newPos.y / static_cast<float>(ts.y));
    if (!map_.isCollidable(tileX, tileY)) {
        hero_.setPosition(newPos);
    }
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
