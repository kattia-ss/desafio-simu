#include <SFML/Graphics.hpp>
#include <vector>
#include <iostream>
#include <set>
#include <string> 

#include "GameConstants.h"
#include "HexagonCell.h"
#include "Player.h"
#include "MapLoader.h"

using namespace sf;
using namespace std;

int main() {
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

    if (grid.empty() || grid[0].empty()) {
        cerr << "Error: El mapa cargado está vacío.\n";
        return -1;
    }

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
        cerr << "Error: celda START no encontrada en el mapa.\n";
        return -1;
    }

    // Calculate offsets for centering the grid
    float totalGridWidth = (GRID_COLS - 1) * (HEX_APOTHEM * 2 + HEX_SPACING) + HEX_APOTHEM * 2;
    float totalGridHeight = (GRID_ROWS - 1) * (HEX_RADIUS * 1.5f + HEX_SPACING) + HEX_RADIUS * 2;
    float offsetX = (WINDOW_WIDTH - totalGridWidth) / 2.f;
    float offsetY = (WINDOW_HEIGHT - totalGridHeight) / 2.f;

    for (int r = 0; r < GRID_ROWS; ++r) {
        for (int c = 0; c < GRID_COLS; ++c) {
            grid[r][c].setScreenPosition(c, r, offsetX, offsetY);
        }
    }

    int waterLevel = -1;
    Clock waterClock;

    set<pair<int, int>> visited;
    // Mark the starting cell as visited
    visited.insert({ player.row, player.col });

    int score = INITIAL_SCORE;
    int energy = 0;
    bool wallBreakUsed = false;

    while (window.isOpen()) {
        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::KeyPressed) {
                int newRow = player.row;
                int newCol = player.col;

                // Handle movement based on keyboard input
                if (event.key.code == Keyboard::W) newRow--;
                if (event.key.code == Keyboard::S) newRow++;
                if (event.key.code == Keyboard::A) newCol--;
                if (event.key.code == Keyboard::D) newCol++;

                // Check for valid moves
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
                        grid[newRow][newCol].setFillColor(Color::White); // Change color after picking up
                    }

                    player.row = newRow;
                    player.col = newCol;

                    if (grid[newRow][newCol].isGoal) {
                        cout << "¡Ganaste! Puntaje final: " << score << "\n";
                        window.close();
                    }
                }

                // Handle wall breaking ability
                if (event.key.code == Keyboard::E && energy == MAX_ENERGY && !wallBreakUsed) {
                    // Directions for adjacent cells (axial coordinates simplified for grid)
                    // These are relative changes in (row, col) for 4-directional movement
                    vector<pair<int, int>> dirs = { {-1, 0}, {1, 0}, {0, -1}, {0, 1} };

                    // For hexagonal grids, the neighbors are more complex. 
                    // Given the current movement (W, S, A, D) implies a square-like movement 
                    // on the hexagonal grid representation, we'll assume a 4-directional check
                    // for wall breaking for simplicity. If a true hexagonal neighbor check is needed,
                    // the `dirs` array would need to be updated with axial or offset coordinates.

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
                            energy = 0; // Reset energy after use
                            break;      // Break only one wall
                        }
                    }
                }
            }

            if (event.type == Event::Closed ||
                (event.type == Event::KeyPressed && event.key.code == Keyboard::Escape)) {
                window.close();
            }
        }

        // Update water level
        if (waterClock.getElapsedTime().asSeconds() > WATER_STEP_INTERVAL) {
            waterLevel++;
            for (int r = 0; r < GRID_ROWS; ++r) {
                for (int c = 0; c < GRID_COLS; ++c) {
                    if (!grid[r][c].isWall && grid[r][c].height <= waterLevel) {
                        grid[r][c].isFlooded = true;
                        grid[r][c].setFillColor(Color(100, 100, 255, 150)); // Semi-transparent blue for flooded
                    }
                }
            }
            waterClock.restart();
        }

        // Check if player is flooded
        if (grid[player.row][player.col].isFlooded) {
            cout << "Te ahogaste. Puntaje final: " << score << "\n";
            window.close();
        }

        window.clear(Color(60, 60, 60)); // Dark grey background

        // Draw the grid
        for (int r = 0; r < GRID_ROWS; ++r) {
            for (int c = 0; c < GRID_COLS; ++c) {
                window.draw(grid[r][c]);
            }
        }

        // Draw player highlight
        CircleShape highlight(HEX_RADIUS / 2.f, 6);
        highlight.setFillColor(Color(50, 100, 255, 180)); // Semi-transparent blue
        highlight.setOrigin(highlight.getRadius(), highlight.getRadius());
        highlight.setPosition(grid[player.row][player.col].getPosition());
        window.draw(highlight);

        // Draw energy bar
        float energyRatio = static_cast<float>(energy) / MAX_ENERGY;
        energyBarFill.setSize(Vector2f(120 * energyRatio, 12));
        window.draw(energyBarBack);
        window.draw(energyBarFill);

        window.display();
    }

    return 0;
}