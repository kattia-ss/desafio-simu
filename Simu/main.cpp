#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <fstream>
#include <iostream>
#include <set>
#include "json.hpp"

using namespace sf;
using namespace std;
using json = nlohmann::json;

const float HEX_RADIUS = 30.0f;
const float HEX_APOTHEM = HEX_RADIUS * sqrtf(3.f) / 2.f;
const float HEX_SPACING = 2.0f;
const float OUTLINE_THICKNESS = 1.5f;
const float WATER_STEP_INTERVAL = 4.0f;
const int INITIAL_SCORE = 100;
const int BACKTRACK_PENALTY = 5;
const int MAX_ENERGY = 5;

class HexagonCell : public ConvexShape {
public:
    int q, r;
    int height;
    bool isWall;
    bool isStart;
    bool isGoal;
    bool isItem;
    bool isFlooded = false;
    bool itemCollected = false;

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

    RenderWindow window(VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Templo - Version Final");
    window.setFramerateLimit(60);

    RectangleShape energyBarBack(Vector2f(120, 12));
    energyBarBack.setPosition(10, 10);
    energyBarBack.setFillColor(Color(50, 50, 50));

    RectangleShape energyBarFill(Vector2f(0, 12));
    energyBarFill.setPosition(10, 10);
    energyBarFill.setFillColor(Color(0, 200, 255));

    vector<vector<HexagonCell>> grid;
    if (!loadMapFromJson("mapa2.json", grid)) return -1;

    if (grid.empty() || grid[0].empty()) return -1;

    int GRID_ROWS = grid.size();
    int GRID_COLS = grid[0].size();

    Player player;
    bool startFound = false;
    for (int r = 0; r < GRID_ROWS && !startFound; ++r)
        for (int c = 0; c < GRID_COLS; ++c)
            if (grid[r][c].isStart) {
                player.row = r;
                player.col = c;
                startFound = true;
                break;
            }

    if (!startFound) {
        cerr << "Error: celda START no encontrada.\n";
        return -1;
    }

    float totalGridWidth = (GRID_COLS - 1) * (HEX_APOTHEM * 2 + HEX_SPACING) + HEX_APOTHEM * 2;
    float totalGridHeight = (GRID_ROWS - 1) * (HEX_RADIUS * 1.5f + HEX_SPACING) + HEX_RADIUS * 2;
    float offsetX = (WINDOW_WIDTH - totalGridWidth) / 2.f;
    float offsetY = (WINDOW_HEIGHT - totalGridHeight) / 2.f;

    for (int r = 0; r < GRID_ROWS; ++r)
        for (int c = 0; c < GRID_COLS; ++c)
            grid[r][c].setScreenPosition(c, r, offsetX, offsetY);

    int waterLevel = -1;
    Clock waterClock;

    set<pair<int, int>> visited;
    int score = INITIAL_SCORE;
    int energy = 0;
    bool wallBreakUsed = false;

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

                    pair<int, int> target = { newRow, newCol };

                    if (visited.count(target)) {
                        score -= BACKTRACK_PENALTY;
                    }
                    else {
                        visited.insert(target);
                        energy = min(energy + 1, MAX_ENERGY);
                    }

                    if (grid[newRow][newCol].isItem && !grid[newRow][newCol].itemCollected) {
                        score += 100;
                        grid[newRow][newCol].itemCollected = true;
                        grid[newRow][newCol].isItem = false;
                        grid[newRow][newCol].setFillColor(Color::White);
                    }

                    player.row = newRow;
                    player.col = newCol;

                    if (grid[newRow][newCol].isGoal) {
                        cout << "¡Ganaste! Puntaje final: " << score << "\n";
                        window.close();
                    }
                }

                if (event.key.code == Keyboard::E && energy == MAX_ENERGY && !wallBreakUsed) {
                    vector<pair<int, int>> dirs = { {-1, 0}, {1, 0}, {0, -1}, {0, 1} };
                    for (size_t i = 0; i < dirs.size(); ++i) {
                        int dr = dirs[i].first;
                        int dc = dirs[i].second;
                        int nr = player.row + dr;
                        int nc = player.col + dc;
                        if (nr >= 0 && nr < GRID_ROWS &&
                            nc >= 0 && nc < GRID_COLS &&
                            grid[nr][nc].isWall) {

                            grid[nr][nc].isWall = false;
                            grid[nr][nc].setFillColor(Color::White);
                            wallBreakUsed = true;
                            energy = 0;
                            break;  // solo se rompe una pared
                        }
                    }
                }

            }

            if (event.type == Event::Closed ||
                (event.type == Event::KeyPressed && event.key.code == Keyboard::Escape)) {
                window.close();
            }
        }

        if (waterClock.getElapsedTime().asSeconds() > WATER_STEP_INTERVAL) {
            waterLevel++;
            for (int r = 0; r < GRID_ROWS; ++r)
                for (int c = 0; c < GRID_COLS; ++c)
                    if (!grid[r][c].isWall && grid[r][c].height <= waterLevel) {
                        grid[r][c].isFlooded = true;
                        grid[r][c].setFillColor(Color(100, 100, 255, 150));
                    }
            waterClock.restart();
        }

        if (grid[player.row][player.col].isFlooded) {
            cout << "Te ahogaste. Puntaje final: " << score << "\n";
            window.close();
        }

        window.clear(Color(60, 60, 60));

        for (int r = 0; r < GRID_ROWS; ++r)
            for (int c = 0; c < GRID_COLS; ++c)
                window.draw(grid[r][c]);

        CircleShape highlight(HEX_RADIUS / 2.f, 6);
        highlight.setFillColor(Color(50, 100, 255, 180));
        highlight.setOrigin(highlight.getRadius(), highlight.getRadius());
        highlight.setPosition(grid[player.row][player.col].getPosition());
        window.draw(highlight);

        float energyRatio = static_cast<float>(energy) / MAX_ENERGY;
        energyBarFill.setSize(Vector2f(120 * energyRatio, 12));
        window.draw(energyBarBack);
        window.draw(energyBarFill);

        window.display();
    }

    return 0;
}
