#pragma once

#include "scene.hpp"
#include "map.hpp"
#include "texture_manager.hpp"
#include <SFML/Graphics.hpp>
#include <string>

class MapScene : public Scene {
public:
    explicit MapScene(TextureManager& textures, const std::string& tmxPath = "game/hello-town.tmx");

    void handleEvent(const sf::Event& event) override;
    void update(float deltaTime) override;
    void draw(sf::RenderWindow& window) const override;

private:
    TextureManager& textures_;
    Map map_;
    sf::RectangleShape hero_;
    float moveSpeed_ = 200.f;
};
