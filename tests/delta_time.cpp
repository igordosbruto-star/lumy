#include <gtest/gtest.h>

#include "delta_time.hpp"

TEST(DeltaTime, ClampsAboveMax) {
    const float maxDeltaTime = 1.f / 30.f;
    const float artificialDelta = 0.1f; // > maxDeltaTime
    float clamped = clampDeltaTime(artificialDelta, maxDeltaTime);
    EXPECT_FLOAT_EQ(clamped, maxDeltaTime);
}

