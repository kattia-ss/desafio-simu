#pragma once // Evitar que el cont del archivo se incluya varias veces

#include <SFML/Graphics.hpp>
#include <cmath>

// Hexagon properties
const float HEX_RADIUS = 30.0f;
const float HEX_APOTHEM = HEX_RADIUS * sqrtf(3.f) / 2.f;
const float HEX_SPACING = 2.0f;
const float OUTLINE_THICKNESS = 1.5f;

// Game mechanics constants
const float WATER_STEP_INTERVAL = 5.0f;
const int INITIAL_SCORE = 100;
const int BACKTRACK_PENALTY = 5;
const int MAX_ENERGY = 5;

// Window properties
const int WINDOW_WIDTH = 1000;
const int WINDOW_HEIGHT = 800;