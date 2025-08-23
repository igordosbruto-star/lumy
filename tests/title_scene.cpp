#include <gtest/gtest.h>
#include <filesystem>
#include <stdexcept>

#include "scene_stack.hpp"
#include "title_scene.hpp"
#include "texture_manager.hpp"

namespace {
struct ScopedCurrentPath {
    explicit ScopedCurrentPath(const std::filesystem::path& p)
        : old_(std::filesystem::current_path()) {
        std::filesystem::current_path(p);
    }

    ~ScopedCurrentPath() { std::filesystem::current_path(old_); }

private:
    std::filesystem::path old_;
};
} // namespace

TEST(TitleScene, MissingFontThrows) {
    auto testsDir = std::filesystem::path{__FILE__}.parent_path();
    ScopedCurrentPath change{testsDir};
    SceneStack stack;
    TextureManager textures;
    EXPECT_THROW((TitleScene{stack, textures}), std::runtime_error);
}
