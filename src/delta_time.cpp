#include "delta_time.hpp"

#include <iostream>

float clampDeltaTime(float deltaTime, float maxDeltaTime) {
    if (deltaTime > maxDeltaTime) {
        std::cout << "[Frame drop] deltaTime: " << deltaTime << "s\n";
        return maxDeltaTime;
    }
    return deltaTime;
}

