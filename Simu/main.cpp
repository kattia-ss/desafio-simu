#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <fstream>
#include <iostream>
#include "json.hpp"

using namespace sf;
using namespace std;
using json = nlohmann::json;

// --- Constantes globales ---
const float HEX_RADIUS = 30.0f;
const float HEX_APOTHEM = HEX_RADIUS * sqrtf(3.f) / 2.f;
const float HEX_SPACING = 2.0f;
const float OUTLINE_THICKNESS = 1.5f;
const float WATER_STEP_INTERVAL = 1.5f; // segundos entre subida de agua

// --- Clase Hexágono ---
class HexagonCell : public ConvexShape {
public:
    int q, r;
    int height;
    bool isWall;
    bool isStart;
    bool isGoal;
    bool isItem;
    bool isFlooded = false;

    HexagonCell(float radius = HEX_RADIUS) {
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
    }

    void setScreenPosition(int col, int row, float dX = 0.f, float dY = 0.f) {
        float x = col * (2 * HEX_APOTHEM + HEX_SPACING);
        float y = row * (HEX_RADIUS * 1.5f + HEX_SPACING);
        if (row % 2 != 0) {
            x += HEX_APOTHEM + HEX_SPACING / 2.f;
        }
        setPosition(Vector2f(x + dX, y + dY));
    }
};

struct Player {
    int row, col;
    Player(int r = 0, int c = 0) : row(r), col(c) {}
};

bool loadMapFromJson(const string& filename, vector<vector<HexagonCell>>& grid) {
    ifstream inFile(filename);
    if (!inFile.is_open()) {
        cerr << "No se pudo abrir el archivo: " << filename << "\n";
        return false;
    }

    json j;
    inFile >> j;

    int rows = j["rows"];
    int cols = j["cols"];
    auto mapGrid = j["grid"];
    auto heightMap = j["heightMap"];

    grid.resize(rows, vector<HexagonCell>(cols));

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            char symbol = mapGrid[r][c].get<string>()[0];
            int height = heightMap[r][c];

            HexagonCell cell;
            cell.q = c;
            cell.r = r;
            cell.height = height;

            switch (symbol) {
            case 'S':
                cell.setFillColor(Color::Green);
                cell.isStart = true;
                break;
            case 'G':
                cell.setFillColor(Color::Red);
                cell.isGoal = true;
                break;
            case 'K':
                cell.setFillColor(Color::Yellow);
                cell.isItem = true;
                break;
            case '#':
                cell.setFillColor(Color(100, 100, 100));
                cell.isWall = true;
                break;
            case '.':
            default:
                cell.setFillColor(Color::White);
                break;
            }

            grid[r][c] = cell;
        }
    }

    return true;
}

int main() {
    const int WINDOW_WIDTH = 1000;
    const int WINDOW_HEIGHT = 800;

    RenderWindow window(VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Templo - Agua en Ascenso");
    window.setFramerateLimit(60);

    vector<vector<HexagonCell>> grid;
    if (!loadMapFromJson("mapa2.json", grid)) return -1;

    int GRID_ROWS = grid.size();
    int GRID_COLS = grid[0].size();

    Player player;
    for (int r = 0; r < GRID_ROWS; ++r)
        for (int c = 0; c < GRID_COLS; ++c)
            if (grid[r][c].isStart) {
                player.row = r;
                player.col = c;
                break;
            }

    float effectiveHexWidth = HEX_APOTHEM * 2.f;
    float effectiveHexHeight = HEX_RADIUS * 2.f;
    float totalGridWidth = (GRID_COLS - 1) * (effectiveHexWidth + HEX_SPACING) + effectiveHexWidth;
    float totalGridHeight = (GRID_ROWS - 1) * (HEX_RADIUS * 1.5f + HEX_SPACING) + effectiveHexHeight;
    float offsetX = (WINDOW_WIDTH - totalGridWidth) / 2.f;
    float offsetY = (WINDOW_HEIGHT - totalGridHeight) / 2.f;

    for (int r = 0; r < GRID_ROWS; ++r)
        for (int c = 0; c < GRID_COLS; ++c)
            grid[r][c].setScreenPosition(c, r, offsetX, offsetY);

    int waterLevel = -1;
    Clock waterClock;

    while (window.isOpen()) {
        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::KeyPressed) {
                int newRow = player.row;
                int newCol = player.col;
                if (event.key.code == Keyboard::W) newRow--;
                if (event.key.code == Keyboard::S) newRow++;
                if (event.key.code == Keyboard::A) newCol--;
                if (event.key.code == Keyboard::D) newCol++;

                if (newRow >= 0 && newRow < GRID_ROWS &&
                    newCol >= 0 && newCol < GRID_COLS &&
                    !grid[newRow][newCol].isWall &&
                    !grid[newRow][newCol].isFlooded) {
                    player.row = newRow;
                    player.col = newCol;
                }
            }
            if (event.type == Event::Closed ||
                (event.type == Event::KeyPressed && event.key.code == Keyboard::Escape)) {
                window.close();
            }
        }

        // Subida de agua cada cierto tiempo
        if (waterClock.getElapsedTime().asSeconds() > WATER_STEP_INTERVAL) {
            waterLevel++;
            for (int r = 0; r < GRID_ROWS; ++r) {
                for (int c = 0; c < GRID_COLS; ++c) {
                    if (!grid[r][c].isWall && grid[r][c].height <= waterLevel) {
                        grid[r][c].isFlooded = true;
                    }
                }
            }
            waterClock.restart();
        }

        window.clear(Color(60, 60, 60));

        for (int r = 0; r < GRID_ROWS; ++r) {
            for (int c = 0; c < GRID_COLS; ++c) {
                if (grid[r][c].isFlooded) {
                    grid[r][c].setFillColor(Color(100, 100, 255, 150));
                }
                window.draw(grid[r][c]);
            }
        }

        CircleShape highlight(HEX_RADIUS / 2.f, 6);
        highlight.setFillColor(Color(50, 100, 255, 180));
        highlight.setOrigin(highlight.getRadius(), highlight.getRadius());
        highlight.setPosition(grid[player.row][player.col].getPosition());
        window.draw(highlight);

        window.display();
    }

    return 0;
}
