#include "scene_stack.hpp"

void SceneStack::pushScene(std::unique_ptr<Scene> scene) {
    pendingActions_.push_back(PendingAction{ActionType::Push, std::move(scene)});
}

void SceneStack::popScene() {
    pendingActions_.push_back(PendingAction{ActionType::Pop, nullptr});
}

void SceneStack::switchScene(std::unique_ptr<Scene> scene) {
    pendingActions_.push_back(PendingAction{ActionType::Switch, std::move(scene)});
}

void SceneStack::applyPending() {
    for (auto& action : pendingActions_) {
        switch (action.type) {
        case ActionType::Push:
            stack_.push_back(std::move(action.scene));
            break;
        case ActionType::Pop:
            if (!stack_.empty()) {
                stack_.pop_back();
            }
            break;
        case ActionType::Switch:
            if (!stack_.empty()) {
                stack_.back() = std::move(action.scene);
            } else {
                stack_.push_back(std::move(action.scene));
            }
            break;
        }
    }
    pendingActions_.clear();
}

Scene* SceneStack::current() const {
    return stack_.empty() ? nullptr : stack_.back().get();
}

