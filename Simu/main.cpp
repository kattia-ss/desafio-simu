#include <SFML/Graphics.hpp>
#include <vector>
#include <iostream>
#include <set>
#include <queue>
#include <cmath>
#include <algorithm>

#include "GameConstants.h"
#include "HexagonCell.h"
#include "Player.h"
#include "MapLoader.h"

using namespace sf;
using namespace std;

struct Node {
    int row, col;
    float gCost, hCost;
    Node* parent;

    float fCost() const { return gCost + hCost; }

    bool operator>(const Node& other) const {
        return fCost() > other.fCost();
    }
};

float heuristic(int r1, int c1, int r2, int c2) {
    return abs(r1 - r2) + abs(c1 - c2); // hex-friendly Manhattan
}

vector<pair<int, int>> getNeighbors(int row, int col, int maxRows, int maxCols) {
    vector<pair<int, int>> neighbors;
    vector<pair<int, int>> offsets = { {-1,0}, {1,0}, {0,-1}, {0,1}, {-1,1}, {1,-1} };
    for (auto offset : offsets) {
        int dr = offset.first;
        int dc = offset.second;
        int nr = row + dr, nc = col + dc;
        if (nr >= 0 && nr < maxRows && nc >= 0 && nc < maxCols)
            neighbors.emplace_back(nr, nc);
    }
    return neighbors;
}

vector<pair<int, int>> findPathAStar(vector<vector<HexagonCell>>& grid, int startR, int startC, int goalR, int goalC) {
    int rows = grid.size(), cols = grid[0].size();
    priority_queue<Node, vector<Node>, greater<Node>> openSet;
    vector<vector<bool>> closed(rows, vector<bool>(cols, false));
    vector<vector<Node*>> nodes(rows, vector<Node*>(cols, nullptr));

    Node* start = new Node{ startR, startC, 0, heuristic(startR, startC, goalR, goalC), nullptr };
    openSet.push(*start);
    nodes[startR][startC] = start;

    while (!openSet.empty()) {
        Node current = openSet.top(); openSet.pop();

        if (current.row == goalR && current.col == goalC) {
            vector<pair<int, int>> path;
            Node* n = nodes[goalR][goalC];
            while (n) {
                path.emplace_back(n->row, n->col);
                n = n->parent;
            }
            reverse(path.begin(), path.end());
            return path;
        }

        closed[current.row][current.col] = true;

        for (auto it : getNeighbors(current.row, current.col, rows, cols)) {
            int nr = it.first;
            int nc = it.second;
            if (closed[nr][nc] || grid[nr][nc].isWall || grid[nr][nc].isFlooded) continue;

            float gCost = current.gCost + 1;
            float hCost = heuristic(nr, nc, goalR, goalC);

            if (!nodes[nr][nc] || gCost < nodes[nr][nc]->gCost) {
                Node* neighbor = new Node{ nr, nc, gCost, hCost, nodes[current.row][current.col] };
                nodes[nr][nc] = neighbor;
                openSet.push(*neighbor);
            }
        }
    }

    return {};
}

int main() {
    RenderWindow window(VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Templo - A* Pathfinding");
    window.setFramerateLimit(60);

    RectangleShape energyBarBack(Vector2f(120, 12));
    energyBarBack.setPosition(10, 10);
    energyBarBack.setFillColor(Color(50, 50, 50));

    RectangleShape energyBarFill(Vector2f(0, 12));
    energyBarFill.setPosition(10, 10);
    energyBarFill.setFillColor(Color(0, 200, 255));

    vector<vector<HexagonCell>> grid;
    if (!loadMapFromJson("mapa2.json", grid)) return -1;

    int GRID_ROWS = grid.size();
    int GRID_COLS = grid[0].size();

    Player player;
    bool startFound = false;
    for (int r = 0; r < GRID_ROWS && !startFound; ++r) {
        for (int c = 0; c < GRID_COLS; ++c) {
            if (grid[r][c].isStart) {
                player.row = r;
                player.col = c;
                startFound = true;
                break;
            }
        }
    }

    if (!startFound) {
        cerr << "START cell not found.\n";
        return -1;
    }

    float totalGridWidth = (GRID_COLS - 1) * (HEX_APOTHEM * 2 + HEX_SPACING) + HEX_APOTHEM * 2;
    float totalGridHeight = (GRID_ROWS - 1) * (HEX_RADIUS * 1.5f + HEX_SPACING) + HEX_RADIUS * 2;
    float offsetX = (WINDOW_WIDTH - totalGridWidth) / 2.f;
    float offsetY = (WINDOW_HEIGHT - totalGridHeight) / 2.f;

    for (int r = 0; r < GRID_ROWS; ++r) {
        for (int c = 0; c < GRID_COLS; ++c) {
            grid[r][c].setScreenPosition(c, r, offsetX, offsetY);

            // Colorear hexágonos según su altura para mostrar los niveles visualmente
            if (!grid[r][c].isWall && !grid[r][c].isStart && !grid[r][c].isGoal && !grid[r][c].isItem) {
                int height = grid[r][c].height;
                // Gradiente de verde claro a verde oscuro según la altura
                int greenValue = 100 + (height * 30); // Más altura = más verde
                if (greenValue > 255) greenValue = 255;
                grid[r][c].setFillColor(Color(50, greenValue, 50));
            }
        }
    }

    Clock waterClock;
    set<pair<int, int>> visited;
    visited.insert({ player.row, player.col });

    int score = INITIAL_SCORE, energy = 0;
    bool wallBreakUsed = false;
    vector<pair<int, int>> currentPath;

    while (window.isOpen()) {
        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::KeyPressed) {
                if (event.key.code == Keyboard::F) {
                    for (int r = 0; r < GRID_ROWS; ++r) {
                        for (int c = 0; c < GRID_COLS; ++c) {
                            if (grid[r][c].isGoal) {
                                currentPath = findPathAStar(grid, player.row, player.col, r, c);
                            }
                        }
                    }
                }

                int newRow = player.row, newCol = player.col;
                if (event.key.code == Keyboard::W) newRow--;
                if (event.key.code == Keyboard::S) newRow++;
                if (event.key.code == Keyboard::A) newCol--;
                if (event.key.code == Keyboard::D) newCol++;

                if (newRow >= 0 && newRow < GRID_ROWS &&
                    newCol >= 0 && newCol < GRID_COLS &&
                    !grid[newRow][newCol].isWall &&
                    !grid[newRow][newCol].isFlooded) {

                    pair<int, int> target = { newRow, newCol };
                    if (!visited.insert(target).second) score -= BACKTRACK_PENALTY;
                    else energy = min(energy + 1, MAX_ENERGY);

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
                    vector<pair<int, int>> dirs = { {-1,0}, {1,0}, {0,-1}, {0,1} };
                    for (auto it : dirs) {
                        int dr = it.first;
                        int dc = it.second;
                        int nr = player.row + dr, nc = player.col + dc;
                        if (nr >= 0 && nr < GRID_ROWS && nc >= 0 && nc < GRID_COLS && grid[nr][nc].isWall) {
                            grid[nr][nc].isWall = false;
                            grid[nr][nc].setFillColor(Color::White);
                            wallBreakUsed = true;
                            energy = 0;
                            break;
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
            static int waterLevel = -1;
            waterLevel++;

            // Encontrar la altura máxima del objetivo (hexágono rojo) para que sea el último en inundarse
            int maxGoalHeight = -1;
            for (int r = 0; r < GRID_ROWS; ++r) {
                for (int c = 0; c < GRID_COLS; ++c) {
                    if (grid[r][c].isGoal) {
                        maxGoalHeight = max(maxGoalHeight, grid[r][c].height);
                    }
                }
            }

            for (int r = 0; r < GRID_ROWS; ++r) {
                for (int c = 0; c < GRID_COLS; ++c) {
                    // No inundar muros ni el objetivo (hexágono rojo) hasta el final
                    if (!grid[r][c].isWall && !grid[r][c].isGoal && grid[r][c].height <= waterLevel) {
                        grid[r][c].isFlooded = true;
                        grid[r][c].setFillColor(Color(100, 100, 255, 150));
                    }
                    // Solo inundar el objetivo si el agua ha alcanzado su altura máxima + margen extra
                    else if (grid[r][c].isGoal && waterLevel > maxGoalHeight + 2) {
                        grid[r][c].isFlooded = true;
                        grid[r][c].setFillColor(Color(100, 100, 255, 150));
                    }
                }
            }
            waterClock.restart();
        }

        if (grid[player.row][player.col].isFlooded) {
            cout << "Te ahogaste. Puntaje final: " << score << "\n";
            window.close();
        }

        window.clear(Color(60, 60, 60));

        for (auto it : currentPath) {
            int r = it.first;
            int c = it.second;
            if (!grid[r][c].isStart && !grid[r][c].isGoal)
                grid[r][c].setFillColor(Color(255, 165, 0)); // Naranja
        }

        for (int r = 0; r < GRID_ROWS; ++r) {
            for (int c = 0; c < GRID_COLS; ++c) {
                window.draw(grid[r][c]);
            }
        }

        CircleShape highlight(HEX_RADIUS / 2.f, 6);
        highlight.setFillColor(Color(50, 100, 255, 180));
        highlight.setOrigin(highlight.getRadius(), highlight.getRadius());
        highlight.setPosition(grid[player.row][player.col].getPosition());
        window.draw(highlight);

        energyBarFill.setSize(Vector2f(120 * (static_cast<float>(energy) / MAX_ENERGY), 12));
        window.draw(energyBarBack);
        window.draw(energyBarFill);

        window.display();
    }

    return 0;
}