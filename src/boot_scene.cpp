#include "boot_scene.hpp"
#include <iostream>

BootScene::BootScene(SceneStack& stack) : stack_(stack) {
    map_.load("game/first.tmx");
}

void BootScene::handleEvent(const sf::Event&) {}

void BootScene::update(float) {
    if (!loaded_) {
        std::cout << "BootScene: loading core resources...\n";
        loaded_ = true;
    }
}

void BootScene::draw(sf::RenderWindow& target) const {
    map_.draw(target);
}

