#include <gtest/gtest.h>
#include <SFML/Graphics.hpp>

#include "scene_stack.hpp"

namespace {
class CountingScene : public Scene {
public:
    int updates = 0;
    void handleEvent(const sf::Event&) override {}
    void update(float) override { ++updates; }
    void draw(sf::RenderWindow&) const override {}
};
} // namespace

TEST(SceneLoop, SkipUpdateAfterClose) {
    sf::RenderWindow window(sf::VideoMode({100, 100}), "Test");
    SceneStack stack;

    auto scene = std::make_unique<CountingScene>();
    CountingScene* scenePtr = scene.get();
    stack.pushScene(std::move(scene));
    stack.applyPending();

    sf::Event event = sf::Event::Closed{};
    if (event.is<sf::Event::Closed>()) {
        window.close();
    }
    if (auto* current = stack.current()) {
        current->handleEvent(event);
    }

    stack.applyPending();

    if (window.isOpen()) {
        if (auto* current = stack.current()) {
            current->update(0.f);
        }
    }

    EXPECT_FALSE(window.isOpen());
    EXPECT_EQ(scenePtr->updates, 0);
}

