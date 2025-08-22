#include <gtest/gtest.h>
#include <SFML/Graphics.hpp>

TEST(BasicStartup, CanCreateWindow) {
    sf::RenderWindow window(sf::VideoMode({100, 100}), "Test");
    EXPECT_TRUE(window.isOpen());
    window.close();
}
