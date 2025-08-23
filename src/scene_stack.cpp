#include "scene_stack.hpp"

void SceneStack::pushScene(std::unique_ptr<Scene> scene) {
    stack_.push_back(std::move(scene));
}

void SceneStack::popScene() {
    if (!stack_.empty()) {
        stack_.pop_back();
    }
}

void SceneStack::switchScene(std::unique_ptr<Scene> scene) {
    if (!stack_.empty()) {
        stack_.back() = std::move(scene);
    } else {
        stack_.push_back(std::move(scene));
    }
}

Scene* SceneStack::current() const {
    return stack_.empty() ? nullptr : stack_.back().get();
}

