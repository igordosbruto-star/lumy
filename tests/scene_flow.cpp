#include <gtest/gtest.h>
#include <memory>

#include "boot_scene.hpp"
#include "map_scene.hpp"
#include "scene_stack.hpp"
#include "title_scene.hpp"

TEST(SceneFlow, BootTitleMap) {
    SceneStack stack;
    stack.pushScene(std::make_unique<BootScene>(stack));
    EXPECT_NE(dynamic_cast<BootScene*>(stack.current()), nullptr);

    stack.current()->update(0.f);
    EXPECT_NE(dynamic_cast<TitleScene*>(stack.current()), nullptr);

    const sf::Event event = sf::Event::KeyPressed{sf::Keyboard::Key::Enter};
    stack.current()->handleEvent(event);
    EXPECT_NE(dynamic_cast<MapScene*>(stack.current()), nullptr);
}

