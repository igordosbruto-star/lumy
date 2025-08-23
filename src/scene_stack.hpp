#pragma once

#include <memory>
#include <vector>

#include "scene.hpp"

class SceneStack {
public:
    enum class ActionType { Push, Pop, Switch };

    struct PendingAction {
        ActionType type;
        std::unique_ptr<Scene> scene;
    };

    void pushScene(std::unique_ptr<Scene> scene);
    void popScene();
    void switchScene(std::unique_ptr<Scene> scene);
    Scene* current() const;
    void applyPending();

private:
    std::vector<std::unique_ptr<Scene>> stack_;
    std::vector<PendingAction> pendingActions_;
};

