#include <gtest/gtest.h>
#include "scene_stack.hpp"

namespace {
class DummyScene : public Scene {
public:
    void handleEvent(const sf::Event&) override {}
    void update(float) override {}
    void draw(sf::RenderWindow&) const override {}
};
} // namespace

TEST(SceneStack, PushPopSwitch) {
    SceneStack stack;

    auto first = std::make_unique<DummyScene>();
    Scene* firstPtr = first.get();
    stack.pushScene(std::move(first));
    EXPECT_EQ(stack.current(), firstPtr);

    auto second = std::make_unique<DummyScene>();
    Scene* secondPtr = second.get();
    stack.pushScene(std::move(second));
    EXPECT_EQ(stack.current(), secondPtr);

    stack.popScene();
    EXPECT_EQ(stack.current(), firstPtr);

    stack.popScene();
    EXPECT_EQ(stack.current(), nullptr);

    stack.popScene();
    EXPECT_EQ(stack.current(), nullptr);

    auto third = std::make_unique<DummyScene>();
    Scene* thirdPtr = third.get();
    stack.switchScene(std::move(third));
    EXPECT_EQ(stack.current(), thirdPtr);

    auto fourth = std::make_unique<DummyScene>();
    Scene* fourthPtr = fourth.get();
    stack.switchScene(std::move(fourth));
    EXPECT_EQ(stack.current(), fourthPtr);
}

