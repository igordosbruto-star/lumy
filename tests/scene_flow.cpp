#include <gtest/gtest.h>
#include <memory>

#include "boot_scene.hpp"
#include "map_scene.hpp"
#include "scene_stack.hpp"
#include "title_scene.hpp"
#include "texture_manager.hpp"

TEST(SceneFlow, BootTitleMap) {
    TextureManager textures;
    SceneStack stack;
    stack.pushScene(std::make_unique<BootScene>(stack, textures));
    EXPECT_EQ(stack.current(), nullptr);
    stack.applyPending();
    EXPECT_NE(dynamic_cast<BootScene*>(stack.current()), nullptr);

    stack.current()->update(0.f);
    EXPECT_NE(dynamic_cast<BootScene*>(stack.current()), nullptr);
    stack.applyPending();
    EXPECT_NE(dynamic_cast<TitleScene*>(stack.current()), nullptr);

    const sf::Event event = sf::Event::KeyPressed{sf::Keyboard::Key::Enter};
    stack.current()->handleEvent(event);
    EXPECT_NE(dynamic_cast<TitleScene*>(stack.current()), nullptr);
    stack.applyPending();
    EXPECT_NE(dynamic_cast<MapScene*>(stack.current()), nullptr);

    stack.switchScene(std::make_unique<TitleScene>(stack));
    stack.applyPending();
    EXPECT_NE(dynamic_cast<TitleScene*>(stack.current()), nullptr);

    const sf::Event again = sf::Event::KeyPressed{sf::Keyboard::Key::Enter};
    stack.current()->handleEvent(again);
    EXPECT_NE(dynamic_cast<TitleScene*>(stack.current()), nullptr);
    stack.applyPending();
    EXPECT_NE(dynamic_cast<MapScene*>(stack.current()), nullptr);
}

