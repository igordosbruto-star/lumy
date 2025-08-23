#include <gtest/gtest.h>
#include <filesystem>
#include <stdexcept>

#include "scene_stack.hpp"
#include "title_scene.hpp"
#include "texture_manager.hpp"

TEST(TitleScene, MissingFontThrows) {
    auto old = std::filesystem::current_path();
    auto testsDir = std::filesystem::path{__FILE__}.parent_path();
    std::filesystem::current_path(testsDir);
    SceneStack stack;
    TextureManager textures;
    EXPECT_THROW((TitleScene{stack, textures}), std::runtime_error);
    std::filesystem::current_path(old);
}
