#include "boot_scene.hpp"
#include "title_scene.hpp"
#include <iostream>
#include <memory>

BootScene::BootScene(SceneStack& stack) : stack_(stack) {}

void BootScene::handleEvent(const sf::Event&) {}

void BootScene::update(float) {
    if (!loaded_) {
        std::cout << "BootScene: loading core resources...\n";
        loaded_ = true;
        stack_.switchScene(std::make_unique<TitleScene>(stack_));
    }
}

void BootScene::draw(sf::RenderWindow&) const {}

