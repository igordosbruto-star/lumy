#include <gtest/gtest.h>
#include "scene_stack.hpp"

TEST(SceneStack, PushPopSwitch) {
    SceneStack stack;

    auto first = std::make_unique<Scene>(sf::Vector2f{0.f, 0.f});
    Scene* firstPtr = first.get();
    stack.pushScene(std::move(first));
    EXPECT_EQ(stack.current(), firstPtr);

    auto second = std::make_unique<Scene>(sf::Vector2f{1.f, 1.f});
    Scene* secondPtr = second.get();
    stack.pushScene(std::move(second));
    EXPECT_EQ(stack.current(), secondPtr);

    stack.popScene();
    EXPECT_EQ(stack.current(), firstPtr);

    stack.popScene();
    EXPECT_EQ(stack.current(), nullptr);

    stack.popScene();
    EXPECT_EQ(stack.current(), nullptr);

    auto third = std::make_unique<Scene>(sf::Vector2f{2.f, 2.f});
    Scene* thirdPtr = third.get();
    stack.switchScene(std::move(third));
    EXPECT_EQ(stack.current(), thirdPtr);

    auto fourth = std::make_unique<Scene>(sf::Vector2f{3.f, 3.f});
    Scene* fourthPtr = fourth.get();
    stack.switchScene(std::move(fourth));
    EXPECT_EQ(stack.current(), fourthPtr);
}

