#pragma once

#include "scene.hpp"
#include "map.hpp"
#include "texture_manager.hpp"
#include "event_system.hpp"
#include "save_system.hpp"
#include <SFML/Graphics.hpp>
#include <string>
#include <memory>
#include <optional>

class MapScene : public Scene {
public:
    explicit MapScene(SceneStack& stack, TextureManager& textures, const std::string& tmxPath = "game/assets/maps/hello.tmx");

    void handleEvent(const sf::Event& event) override;
    void update(float deltaTime) override;
    void draw(sf::RenderWindow& window) const override;

private:
    void setupExampleEvents();
    void checkEventTriggers();
    
    SceneStack& sceneStack_;
    TextureManager& textures_;
    Map map_;
    sf::RectangleShape hero_;
    float moveSpeed_ = 200.f;
    
    std::unique_ptr<EventSystem> eventSystem_;
    std::unique_ptr<SaveSystem> saveSystem_;
    
    bool showingUI_ = false;
    sf::Font uiFont_;
    std::optional<sf::Text> uiText_;
};
