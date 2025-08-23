#pragma once

// Retorna deltaTime limitado a maxDeltaTime.
// Loga queda de frame quando deltaTime excede o limite.
float clampDeltaTime(float deltaTime, float maxDeltaTime);

