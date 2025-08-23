#pragma once

#include <memory>
#include <vector>

#include "scene.hpp"

class SceneStack {
public:
    void pushScene(std::unique_ptr<Scene> scene);
    void popScene();
    void switchScene(std::unique_ptr<Scene> scene);
    Scene* current() const;

private:
    std::vector<std::unique_ptr<Scene>> stack_;
};

