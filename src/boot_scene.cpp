#include "boot_scene.hpp"
#include "title_scene.hpp"
#include <iostream>
#include <memory>

BootScene::BootScene(SceneStack& stack, TextureManager& textures)
    : stack_(stack), textures_(textures), map_(textures_) {
    map_.load("game/first.tmx");
}

void BootScene::handleEvent(const sf::Event&) {}

void BootScene::update(float) {
    if (!loaded_) {
        std::cout << "BootScene: loading core resources...\n";
        loaded_ = true;
        stack_.switchScene(std::make_unique<TitleScene>(stack_, textures_));
    }
}

void BootScene::draw(sf::RenderWindow& target) const {
    map_.draw(target);
}

