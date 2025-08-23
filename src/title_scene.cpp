#include "title_scene.hpp"
#include "map_scene.hpp"
#include <memory>

TitleScene::TitleScene(SceneStack& stack)
    : stack_(stack), startText_(font_, "Start", 32) {
    font_.openFromFile("game/font.ttf");
    startText_.setPosition({200.f, 150.f});
}

void TitleScene::handleEvent(const sf::Event& event) {
    if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
        if (key->code == sf::Keyboard::Key::Enter) {
            stack_.switchScene(std::make_unique<MapScene>(sf::Vector2f{320.f, 180.f}));
        }
    } else if (const auto* mouse = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mouse->button == sf::Mouse::Button::Left) {
            auto bounds = startText_.getGlobalBounds();
            if (bounds.contains(sf::Vector2f{static_cast<float>(mouse->position.x),
                                            static_cast<float>(mouse->position.y)})) {
                stack_.switchScene(std::make_unique<MapScene>(sf::Vector2f{320.f, 180.f}));
            }
        }
    }
}

void TitleScene::update(float) {}

void TitleScene::draw(sf::RenderWindow& window) const {
    window.draw(startText_);
}

