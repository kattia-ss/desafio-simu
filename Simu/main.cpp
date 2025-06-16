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

            // Calcular desplazamientos para centrar la cuadrícula
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
            // Marcar la celda inicial como visitada
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

                        // Manejar movimiento basado en la entrada del teclado
                        if (event.key.code == Keyboard::W) newRow--;
                        if (event.key.code == Keyboard::S) newRow++;
                        if (event.key.code == Keyboard::A) newCol--;
                        if (event.key.code == Keyboard::D) newCol++;

                        // Verificar movimientos válidos
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
                                grid[newRow][newCol].setFillColor(Color::White); // Cambiar color después de recoger el objeto
                            }

                            player.row = newRow;
                            player.col = newCol;

                            if (grid[newRow][newCol].isGoal) {
                                cout << "¡Ganaste! Puntaje final: " << score << "\n";
                                window.close();
                            }
                        }

                        // Manejar la habilidad de romper paredes
                        if (event.key.code == Keyboard::E && energy == MAX_ENERGY && !wallBreakUsed) {
                            // Estos son cambios relativos en (fila, columna) para movimiento en 4 direcciones
                            vector<pair<int, int>> dirs = { {-1, 0}, {1, 0}, {0, -1}, {0, 1} };

                            // Para cuadrículas hexagonales, los vecinos son más complejos.
                            // Dado que el movimiento actual (W, S, A, D) implica un movimiento similar a cuadrado
                            // en la representación de la cuadrícula hexagonal, asumiremos una verificación de 4 direcciones
                            // para romper paredes por simplicidad. Si se necesita una verdadera verificación de vecinos hexagonales,
                            // el array `dirs` debería actualizarse con coordenadas axiales u offset.

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
                                    energy = 0; // Restablecer energía después de usarla
                                    break;      // Romper solo una pared
                                }
                            }
                        }
                    }

                    if (event.type == Event::Closed ||
                        (event.type == Event::KeyPressed && event.key.code == Keyboard::Escape)) {
                        window.close();
                    }
                }

                // Actualizar nivel de agua
                if (waterClock.getElapsedTime().asSeconds() > WATER_STEP_INTERVAL) {
                    waterLevel++;
                    for (int r = 0; r < GRID_ROWS; ++r) {
                        for (int c = 0; c < GRID_COLS; ++c) {
                            if (!grid[r][c].isWall && grid[r][c].height <= waterLevel) {
                                grid[r][c].isFlooded = true;
                                grid[r][c].setFillColor(Color(100, 100, 255, 150)); // Azul semi-transparente para inundados
                            }
                        }
                    }
                    waterClock.restart();
                }

                // Verificar si el jugador está inundado
                if (grid[player.row][player.col].isFlooded) {
                    cout << "Te ahogaste. Puntaje final: " << score << "\n";
                    window.close();
                }

                window.clear(Color(60, 60, 60)); // Fondo gris oscuro

                // Dibujar la cuadrícula
                for (int r = 0; r < GRID_ROWS; ++r) {
                    for (int c = 0; c < GRID_COLS; ++c) {
                        window.draw(grid[r][c]);
                    }
                }

                // Dibujar resaltado del jugador
                CircleShape highlight(HEX_RADIUS / 2.f, 6);
                highlight.setFillColor(Color(50, 100, 255, 180)); // Azul semi-transparente
                highlight.setOrigin(highlight.getRadius(), highlight.getRadius());
                highlight.setPosition(grid[player.row][player.col].getPosition());
                window.draw(highlight);

                // Dibujar barra de energía
                float energyRatio = static_cast<float>(energy) / MAX_ENERGY;
                energyBarFill.setSize(Vector2f(120 * energyRatio, 12));
                window.draw(energyBarBack);
                window.draw(energyBarFill);

                window.display();
            }

            return 0;
        }