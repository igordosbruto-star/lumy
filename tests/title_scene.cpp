#include <gtest/gtest.h>
#include <filesystem>
#include <stdexcept>

#include "scene_stack.hpp"
#include "title_scene.hpp"

TEST(TitleScene, MissingFontThrows) {
    auto old = std::filesystem::current_path();
    std::filesystem::current_path(old / "tests");
    SceneStack stack;
    EXPECT_THROW(TitleScene{stack}, std::runtime_error);
    std::filesystem::current_path(old);
}