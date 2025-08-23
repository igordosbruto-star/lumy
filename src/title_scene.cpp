#include "title_scene.hpp"
#include "map_scene.hpp"
#include <memory>
#include <stdexcept>

TitleScene::TitleScene(SceneStack& stack)
    : stack_(stack), startText_(font_, "Start", 32) {
    if (!font_.openFromFile("game/font.ttf")) {
        throw std::runtime_error("failed to load font game/font.ttf");
    }
    startText_.setPosition({200.f, 150.f});
}

void TitleScene::handleEvent(const sf::Event& event) {
    if (event.is<sf::Event::KeyPressed>() &&
        event.get<sf::Event::KeyPressed>().code == sf::Keyboard::Key::Enter) {
        stack_.switchScene(std::make_unique<MapScene>(sf::Vector2f{320.f, 180.f}));
    }
}

void TitleScene::update(float) {}

void TitleScene::draw(sf::RenderWindow& window) const {
    window.draw(startText_);
}

