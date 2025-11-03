#include "title_scene.hpp"
#include "map_scene.hpp"
#include <memory>
#include <stdexcept>

TitleScene::TitleScene(SceneStack& stack, TextureManager& textures)
    : stack_(stack), textures_(textures), startText_(font_, "Start", 32) {
    if (!font_.openFromFile("game/font.ttf")) {
        throw std::runtime_error("failed to load font game/font.ttf");
    }
    startText_.setPosition({200.f, 150.f});
}

void TitleScene::handleEvent(const sf::Event& event) {
    if (const auto* key = event.getIf<sf::Event::KeyPressed>();
        key && key->code == sf::Keyboard::Key::Enter) {
        stack_.switchScene(std::make_unique<MapScene>(stack_, textures_));
    }
}

void TitleScene::update(float) {}

void TitleScene::draw(sf::RenderWindow& window) const {
    window.draw(startText_);
}

