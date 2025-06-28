#include "HexagonCell.h"
#include "GameConstants.h" 

HexagonCell::HexagonCell(float radius) {
    setPointCount(6);
    for (int i = 0; i < 6; ++i) {
        float angle_deg = 60.f * i - 30.f;
        float angle_rad = angle_deg * 3.14159265f / 180.f;
        float x = radius * cos(angle_rad);
        float y = radius * sin(angle_rad);
        setPoint(i, Vector2f(x, y));
    }

    setOrigin(0.f, 0.f);
    setFillColor(Color::White);
    setOutlineThickness(OUTLINE_THICKNESS);
    setOutlineColor(Color::Black);

    q = r = height = 0;
    isWall = isStart = isGoal = isItem = false;
    isFlooded = false;
    itemCollected = false;
}

void HexagonCell::setScreenPosition(int col, int row, float dX, float dY) {
    float x = col * (2 * HEX_APOTHEM + HEX_SPACING);
    float y = row * (HEX_RADIUS * 1.5f + HEX_SPACING);
    if (row % 2 != 0) {
        x += HEX_APOTHEM + HEX_SPACING / 2.f;
    }
    setPosition(Vector2f(x + dX, y + dY));
}