#pragma once

#include <SFML/Graphics.hpp>
#include "GameConstants.h" // Constantes del encabezado

using namespace sf;

class HexagonCell : public ConvexShape {
public:
    int q, r;
    int height;
    bool isWall;
    bool isStart;
    bool isGoal;
    bool isItem;
    bool isFlooded;
    bool itemCollected;

    HexagonCell(float radius = HEX_RADIUS);
    void setScreenPosition(int col, int row, float dX = 0.f, float dY = 0.f);
}; 
