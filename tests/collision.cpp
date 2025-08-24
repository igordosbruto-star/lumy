#include <gtest/gtest.h>

#include "map.hpp"
#include "texture_manager.hpp"

TEST(Collision, BlocksMovementIntoWall) {
    TextureManager textures;
    Map map(textures);
    ASSERT_TRUE(map.load("game/assets/maps/hello.tmx"));

    sf::Vector2f pos{0.f, 0.f};
    sf::Vector2f move{1.f, 0.f};
    sf::Vector2f next = pos + move;
    const auto &ts = map.getTileSize();
    unsigned tileX = static_cast<unsigned>(next.x / static_cast<float>(ts.x));
    unsigned tileY = static_cast<unsigned>(next.y / static_cast<float>(ts.y));
    if (!map.isCollidable(tileX, tileY)) {
        pos = next;
    }

    EXPECT_FLOAT_EQ(pos.x, 0.f);
    EXPECT_FLOAT_EQ(pos.y, 0.f);
}
