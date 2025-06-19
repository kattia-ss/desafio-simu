#include <SFML/Graphics.hpp>
#include <vector>
#include <iostream>
#include <set>
#include <queue>
#include <cmath>
#include <algorithm>
#include <functional>
#include <unordered_set>
#include <unordered_map>
#include <fstream>
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

// Función hash para pair<int,int> para usar en unordered_set
struct PairHash {
    size_t operator()(const pair<int, int>& p) const {
        return hash<int>()(p.first) ^ (hash<int>()(p.second) << 1);
    }
};

// Heurística mejorada para hexágonos
float heuristic(int r1, int c1, int r2, int c2) {
    // Convertir coordenadas de hexágono a coordenadas cúbicas para mejor cálculo
    // Esta es una aproximación más precisa para hexágonos
    float x1 = c1 - (r1 - (r1 & 1)) / 2.0f;
    float z1 = r1;
    float y1 = -x1 - z1;

    float x2 = c2 - (r2 - (r2 & 1)) / 2.0f;
    float z2 = r2;
    float y2 = -x2 - z2;

    return (abs(x1 - x2) + abs(y1 - y2) + abs(z1 - z2)) / 2.0f;
}

vector<pair<int, int>> getHexNeighbors(int row, int col, int maxRows, int maxCols) {
    vector<pair<int, int>> neighbors;

    // Corregir los offsets para hexágonos - el problema principal estaba aquí
    static const pair<int, int> evenRowOffsets[6] = {
        {-1, -1}, {-1,  0}, { 0, -1},  // arriba-izq, arriba-der, izquierda
        { 0, +1}, {+1, -1}, {+1,  0}   // derecha, abajo-izq, abajo-der
    };

    static const pair<int, int> oddRowOffsets[6] = {
        {-1,  0}, {-1, +1}, { 0, -1},  // arriba-izq, arriba-der, izquierda
        { 0, +1}, {+1,  0}, {+1, +1}   // derecha, abajo-izq, abajo-der
    };

    const pair<int, int>* offsets = (row % 2 == 0) ? evenRowOffsets : oddRowOffsets;

    for (int i = 0; i < 6; ++i) {
        int nr = row + offsets[i].first;
        int nc = col + offsets[i].second;

        if (nr >= 0 && nr < maxRows && nc >= 0 && nc < maxCols) {
            neighbors.emplace_back(nr, nc);
        }
    }

    return neighbors;
}

vector<pair<int, int>> findPathAStar(vector<vector<HexagonCell>>& grid, int startR, int startC, int goalR, int goalC) {
    int rows = grid.size(), cols = grid[0].size();

    // Usar unordered_set para mejor performance
    unordered_set<pair<int, int>, PairHash> closedSet;
    unordered_map<pair<int, int>, Node*, PairHash> allNodes;

    priority_queue<Node*, vector<Node*>, function<bool(Node*, Node*)>> openSet(
        [](Node* a, Node* b) { return a->fCost() > b->fCost(); }
    );

    Node* start = new Node{ startR, startC, 0, heuristic(startR, startC, goalR, goalC), nullptr };
    openSet.push(start);
    allNodes[{startR, startC}] = start;

    while (!openSet.empty()) {
        Node* current = openSet.top();
        openSet.pop();

        pair<int, int> currentPos = { current->row, current->col };

        // Si ya procesamos este nodo, continuar
        if (closedSet.count(currentPos)) {
            continue;
        }

        // Marcar como procesado
        closedSet.insert(currentPos);

        // ¿Llegamos al objetivo?
        if (current->row == goalR && current->col == goalC) {
            vector<pair<int, int>> path;
            Node* n = current;
            while (n) {
                path.emplace_back(n->row, n->col);
                n = n->parent;
            }
            reverse(path.begin(), path.end());

            // Limpiar memoria
            for (auto& nodePair : allNodes) {
                delete nodePair.second;
            }

            return path;
        }

        // Explorar vecinos
        for (auto neighbor : getHexNeighbors(current->row, current->col, rows, cols)) {
            int nr = neighbor.first;
            int nc = neighbor.second;
            pair<int, int> neighborPos = { nr, nc };

            // Saltar si ya está en el conjunto cerrado
            if (closedSet.count(neighborPos)) continue;

            // Saltar si es una pared o está inundado
            if (grid[nr][nc].isWall || grid[nr][nc].isFlooded) continue;

            float tentativeGCost = current->gCost + 1;

            // Si no hemos visto este nodo antes, o encontramos un camino mejor
            if (allNodes.find(neighborPos) == allNodes.end() ||
                tentativeGCost < allNodes[neighborPos]->gCost) {

                Node* neighborNode;
                if (allNodes.find(neighborPos) == allNodes.end()) {
                    neighborNode = new Node{ nr, nc, tentativeGCost,
                                          heuristic(nr, nc, goalR, goalC), current };
                    allNodes[neighborPos] = neighborNode;
                }
                else {
                    neighborNode = allNodes[neighborPos];
                    neighborNode->gCost = tentativeGCost;
                    neighborNode->parent = current;
                }

                openSet.push(neighborNode);
            }
        }
    }

    // Limpiar memoria si no se encontró camino
    for (auto& nodePair : allNodes) {
        delete nodePair.second;
    }

    return {}; // No se encontró camino
}

//pantalla de derrota
void showEndScreen(sf::RenderWindow& window, const std::string& message, const sf::Font& font) {
    sf::Text endText;
    endText.setFont(font);
    endText.setCharacterSize(32);
    endText.setFillColor(sf::Color::White);
    endText.setString(message + "\n\nPresiona CUALQUIER tecla para salir");

    // Centrar texto
    sf::FloatRect textRect = endText.getLocalBounds();
    endText.setOrigin(textRect.width / 2.f, textRect.height / 2.f);
    endText.setPosition(WINDOW_WIDTH / 2.f, WINDOW_HEIGHT / 2.f);

    window.clear(sf::Color::Black);
    window.draw(endText);
    window.display();

    // Esperar a que el usuario presione una tecla
    sf::Event event;
    while (true) {
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::KeyPressed || event.type == sf::Event::Closed) {
                window.close();
                return;
            }
        }
    }
}



int main() {
    //Para resolver automaticamente
    bool autoMoveEnabled = false;
    size_t autoMoveIndex = 1;  // comienza desde 1 porque 0 es la posición actual del jugador
    Clock autoMoveClock;
    const float AUTO_MOVE_INTERVAL = 0.3f; // segundos entre cada paso


    RenderWindow window(VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Templo - A* Pathfinding Fixed");
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

    // Guardar colores originales para restaurar después
    vector<vector<Color>> originalColors(GRID_ROWS, vector<Color>(GRID_COLS));
    for (int r = 0; r < GRID_ROWS; ++r) {
        for (int c = 0; c < GRID_COLS; ++c) {
            grid[r][c].setScreenPosition(c, r, offsetX, offsetY);

            if (!grid[r][c].isWall && !grid[r][c].isStart && !grid[r][c].isGoal && !grid[r][c].isItem) {
                int height = grid[r][c].height;
                int greenValue = 100 + (height * 30);
                if (greenValue > 255) greenValue = 255;
                Color cellColor = Color(50, greenValue, 50);
                grid[r][c].setFillColor(cellColor);
                originalColors[r][c] = cellColor;
            }
            else {
                originalColors[r][c] = grid[r][c].getFillColor();
            }
        }
    }

    Clock waterClock;
    set<pair<int, int>> visited;
    visited.insert({ player.row, player.col });

    int score = INITIAL_SCORE, energy = 0;
    bool wallBreakUsed = false;
    vector<pair<int, int>> currentPath;
    pair<int, int> lastMoveDir = { 0, 0 };

    // Mostrar puntuación inicial
    cout << "=== TEMPLO - JUEGO INICIADO ===" << endl;
    cout << "Puntuación inicial: " << score << " puntos" << endl;
    cout << "Presiona F para mostrar el camino A*" << endl;
    cout << "===============================" << endl;

    // Intentar cargar la fuente - CORREGIDO
    
    bool fontLoaded = false;
    std::ifstream check("fonts/ARCADECLASSIC.TTF");
    if (!check.is_open()) {
        std::cerr << " No se puede abrir fonts/ARCADECLASSIC.TTF desde C++" << std::endl;
        return -1;
    }

    sf::Font font;
    if (!font.loadFromFile("fonts/ARCADECLASSIC.TTF")) {
        std::cerr << "Error cargando fuente" << std::endl;
        return -1;
    }

    

    // Crear textos solo si la fuente se cargó correctamente
    Text scoreText, visitedText, energyText, positionText, pathText, timeText, instructionsText, automaticMode;
    Clock gameClock; // Para mostrar tiempo transcurrido

    fontLoaded = true;

    if (fontLoaded) {
        // Configurar texto de puntuación
        scoreText.setFont(font);
        scoreText.setCharacterSize(50);
        scoreText.setFillColor(Color::White);
        scoreText.setPosition(10, 30);

        // Configurar texto de celdas visitadas
        visitedText.setFont(font);
        visitedText.setCharacterSize(16);
        visitedText.setFillColor(Color(200, 200, 200));
        visitedText.setPosition(10, 85);

        // Configurar texto de energía
        energyText.setFont(font);
        energyText.setCharacterSize(16);
        energyText.setFillColor(Color(0, 200, 255));
        energyText.setPosition(140, 8);

        // Configurar texto de posición actual
        positionText.setFont(font);
        positionText.setCharacterSize(14);
        positionText.setFillColor(Color(255, 255, 0));
        positionText.setPosition(10, 110);

        // Configurar texto de información del camino
        pathText.setFont(font);
        pathText.setCharacterSize(14);
        pathText.setFillColor(Color(255, 165, 0));
        pathText.setPosition(10, 130);

        // Configurar texto de tiempo
        timeText.setFont(font);
        timeText.setCharacterSize(14);
        timeText.setFillColor(Color(150, 255, 150));
        timeText.setPosition(10, 150);

        //configurar modo automatico
        automaticMode.setFont(font);
        automaticMode.setCharacterSize(14);
        automaticMode.setFillColor(Color::White);
        automaticMode.setPosition(10, 170);


        // Configurar texto de instrucciones
        instructionsText.setFont(font);
        instructionsText.setCharacterSize(30);
        instructionsText.setFillColor(Color(180, 180, 180));
        instructionsText.setPosition(10, WINDOW_HEIGHT - 60);
        instructionsText.setString("Controles: W y E  es arriba A y D  es izq o der  Z y X  abajo \nF Mostrar ruta o R Romper muro energia completa\nESC: Salir");
    }

    while (window.isOpen()) {
        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::KeyPressed) {
                if (event.key.code == Keyboard::F) {
                    // Restaurar colores originales
                    for (int r = 0; r < GRID_ROWS; ++r) {
                        for (int c = 0; c < GRID_COLS; ++c) {
                            if (!grid[r][c].isFlooded) {
                                grid[r][c].setFillColor(originalColors[r][c]);
                            }
                        }
                    }

                    // Encontrar objetivo y calcular ruta
                    for (int r = 0; r < GRID_ROWS; ++r) {
                        for (int c = 0; c < GRID_COLS; ++c) {
                            if (grid[r][c].isGoal) {
                                currentPath = findPathAStar(grid, player.row, player.col, r, c);
                                if (!currentPath.empty()) {
                                    cout << "???  RUTA A* CALCULADA:" << endl;
                                    cout << "   Longitud del camino: " << currentPath.size() << " pasos" << endl;
                                    cout << "   Puntuacion actual: " << score << " puntos" << endl;
                                    cout << "   Celdas visitadas: " << visited.size() << endl;
                                }
                                else {
                                    cout << "? No se encontro ruta al objetivo!" << endl;
                                }
                                break;
                            }
                        }
                    }
                }

                // Movimiento del jugador con offsets corregidos
                int newRow = player.row, newCol = player.col;
                bool isEvenRow = player.row % 2 == 0;

                if (event.key.code == Keyboard::W) {      // Arriba-izquierda
                    newRow = player.row - 1;
                    newCol = isEvenRow ? player.col - 1 : player.col;
                    lastMoveDir = make_pair(-1, isEvenRow ? -1 : 0);
                }
                else if (event.key.code == Keyboard::E) { // Arriba-derecha
                    newRow = player.row - 1;
                    newCol = isEvenRow ? player.col : player.col + 1;
                    lastMoveDir = make_pair(-1, isEvenRow ? 0 : 1);
                }
                else if (event.key.code == Keyboard::A) { // Izquierda
                    newRow = player.row;
                    newCol = player.col - 1;
                    lastMoveDir = make_pair(0, -1);
                }
                else if (event.key.code == Keyboard::D) { // Derecha
                    newRow = player.row;
                    newCol = player.col + 1;
                    lastMoveDir = make_pair(0, 1);
                }
                else if (event.key.code == Keyboard::Z) { // Abajo-izquierda
                    newRow = player.row + 1;
                    newCol = isEvenRow ? player.col - 1 : player.col;
                    lastMoveDir = make_pair(1, isEvenRow ? -1 : 0);
                }
                else if (event.key.code == Keyboard::X) { // Abajo-derecha
                    newRow = player.row + 1;
                    newCol = isEvenRow ? player.col : player.col + 1;
                    lastMoveDir = make_pair(1, isEvenRow ? 0 : 1);
                }

                if (newRow >= 0 && newRow < GRID_ROWS &&
                    newCol >= 0 && newCol < GRID_COLS &&
                    !grid[newRow][newCol].isWall &&
                    !grid[newRow][newCol].isFlooded) {

                    pair<int, int> target = { newRow, newCol };

                    // Verificar si es backtracking (visitando una celda ya visitada)
                    bool isBacktrack = visited.find(target) != visited.end();

                    if (isBacktrack) {
                        score -= BACKTRACK_PENALTY;
                        cout << "?? BACKTRACKING! Penalización: -" << BACKTRACK_PENALTY
                            << " puntos. Puntuacion actual: " << score << endl;
                    }
                    else {
                        visited.insert(target);
                        energy = min(energy + 1, MAX_ENERGY);
                        cout << "? Nueva casilla explorada! Energía: +" << 1
                            << " Puntuacion: " << score << endl;
                    }

                    // Verificar si recogió un item
                    if (grid[newRow][newCol].isItem && !grid[newRow][newCol].itemCollected) {
                        score += 100;
                        grid[newRow][newCol].itemCollected = true;
                        grid[newRow][newCol].isItem = false;
                        grid[newRow][newCol].setFillColor(Color::White);
                        cout << "?? ¡ITEM RECOGIDO! +100 puntos. Puntuacion: " << score << endl;
                    }

                    player.row = newRow;
                    player.col = newCol;

                    if (grid[newRow][newCol].isGoal) {
                        cout << "\n?? ¡GANASTE! ??" << endl;
                        cout << "Puntuacion final: " << score << " puntos" << endl;
                        cout << "Celdas visitadas: " << visited.size() << endl;
                        cout << "Eficiencia: " << (visited.size() > 0 ? (float)score / visited.size() : 0)
                            << " puntos por celda" << endl;
                        showEndScreen(window, "Felicidades ganaste", font);
                    }
                }

                if (event.key.code == Keyboard::R && energy == MAX_ENERGY) {
                    int nr = player.row + lastMoveDir.first;
                    int nc = player.col + lastMoveDir.second;

                    if (grid[nr][nc].isWall) {

                        grid[nr][nc].isWall = false;
                        grid[nr][nc].setFillColor(Color::White);
                        
                        energy = 0;
                        // Establecer altura alta para que  se inunde
                        int goalHeight = 0;
                        for (int r = 0; r < GRID_ROWS; ++r) {
                            for (int c = 0; c < GRID_COLS; ++c) {
                                if (grid[r][c].isGoal) {
                                    goalHeight = grid[r][c].height;
                                }
                            }
                        }
                        grid[nr][nc].height = goalHeight;
                        originalColors[nr][nc] = Color::White;
                        std::cout << "?? Muro roto con altura igual a la meta: " << goalHeight << std::endl;
                    }
                }
                //para resolver automatico cuando presione la letra M 
            } else if (event.key.code == Keyboard::M) {
    // Restaurar colores pero no cuando c rompe un muro obvi duh
                for (int r = 0; r < GRID_ROWS; ++r) {
                    for (int c = 0; c < GRID_COLS; ++c) {
                        if (!grid[r][c].isFlooded && !grid[r][c].isWall) {
                            grid[r][c].setFillColor(originalColors[r][c]);
                        }
                    }
                }

    // Buscar el objetivo
    for (int r = 0; r < GRID_ROWS; ++r) {
        for (int c = 0; c < GRID_COLS; ++c) {
            if (grid[r][c].isGoal) {
                currentPath = findPathAStar(grid, player.row, player.col, r, c);
                autoMoveIndex = 1;
                autoMoveEnabled = !currentPath.empty();
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

        // Sistema de inundación
        if (waterClock.getElapsedTime().asSeconds() > WATER_STEP_INTERVAL) {
            static int waterLevel = -1;
            waterLevel++;

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
                    if (!grid[r][c].isWall && !grid[r][c].isGoal && grid[r][c].height <= waterLevel) {
                        grid[r][c].isFlooded = true;
                        grid[r][c].setFillColor(Color(100, 100, 255, 150));
                    }
                    else if (grid[r][c].isGoal && waterLevel > maxGoalHeight + 2) {
                        grid[r][c].isFlooded = true;
                        grid[r][c].setFillColor(Color(100, 100, 255, 150));
                    }
                }
            }
            waterClock.restart();
        }

        if (grid[player.row][player.col].isFlooded) {
            cout << "\n?? TE AHOGASTE! ??" << endl;
            cout << "Puntuación final: " << score << " puntos" << endl;
            cout << "Celdas visitadas: " << visited.size() << endl;
            cout << "Causa: El agua te alcanzó" << endl;
            showEndScreen(window, "GAME OVER", font);
        }

        //manejador de eventos para la resolucion automatica
        if (autoMoveEnabled && autoMoveIndex < currentPath.size()) {
            if (autoMoveClock.getElapsedTime().asSeconds() >= AUTO_MOVE_INTERVAL) {
                int newRow = currentPath[autoMoveIndex].first;
                int newCol = currentPath[autoMoveIndex].second;

                if (!grid[newRow][newCol].isFlooded) {
                    player.row = newRow;
                    player.col = newCol;
                    visited.insert({ newRow, newCol });
                    energy = min(energy + 1, MAX_ENERGY);

                    if (grid[newRow][newCol].isItem && !grid[newRow][newCol].itemCollected) {
                        score += 100;
                        grid[newRow][newCol].itemCollected = true;
                        grid[newRow][newCol].isItem = false;
                        grid[newRow][newCol].setFillColor(Color::White);
                    }

                    if (grid[newRow][newCol].isGoal) {
                        cout << "\n?? ¡GANASTE! ??" << endl;
                        cout << "Puntuacion final: " << score << " puntos" << endl;
                        cout << "Celdas visitadas: " << visited.size() << endl;
                        cout << "Eficiencia: " << (visited.size() > 0 ? (float)score / visited.size() : 0)
                            << " puntos por celda" << endl;
                        showEndScreen(window, "Felicidades ganaste", font);
                        window.close();
                    }

                    ++autoMoveIndex;
                    autoMoveClock.restart(); // reiniciar el reloj
                }
                else {
                    autoMoveEnabled = false; // detener movimiento si se bloquea
                }
            }
        }

        window.clear(Color(60, 60, 60));

        // Dibujar la cuadrícula primero
        for (int r = 0; r < GRID_ROWS; ++r) {
            for (int c = 0; c < GRID_COLS; ++c) {
                window.draw(grid[r][c]);
            }
        }

        // Colorear la ruta después de dibujar la cuadrícula
        for (size_t i = 0; i < currentPath.size(); ++i) {
            int r = currentPath[i].first;
            int c = currentPath[i].second;

            // No colorear el inicio y el objetivo
            if (!grid[r][c].isStart && !grid[r][c].isGoal && !grid[r][c].isFlooded) {
                // Crear un hexágono temporal para la ruta
                HexagonCell pathCell = grid[r][c];
                pathCell.setFillColor(Color(255, 165, 0, 180)); // Naranja semitransparente
                window.draw(pathCell);
            }
        }

        // Dibujar el jugador
        CircleShape highlight(HEX_RADIUS / 2.f, 6);
        highlight.setFillColor(Color(50, 100, 255, 180));
        highlight.setOrigin(highlight.getRadius(), highlight.getRadius());
        highlight.setPosition(grid[player.row][player.col].getPosition());
        window.draw(highlight);

        // Dibujar la barra de energía
        energyBarFill.setSize(Vector2f(120 * (static_cast<float>(energy) / MAX_ENERGY), 12));
        window.draw(energyBarBack);
        window.draw(energyBarFill);

        // Actualizar y dibujar textos de información solo si la fuente se cargó
        if (fontLoaded) {
            // Actualizar textos con información en tiempo real
            scoreText.setString("Puntuacion " + to_string(score));
            visitedText.setString("Celdas visitadas " + to_string(visited.size()));
            energyText.setString("Energia " + to_string(energy) + " de " + to_string(MAX_ENERGY));
            positionText.setString("Posicion " + to_string(player.row) + "  " + to_string(player.col) + " ");
            automaticMode.setString("Para modo auto presione M");

            // Información del camino A*
            if (!currentPath.empty()) {
                pathText.setString("Ruta con " + to_string(currentPath.size()) + " pasos calculados");
            }
            else {
                pathText.setString("Ruta No calculada presiona F");
            }

            // Tiempo transcurrido
            int timeElapsed = static_cast<int>(gameClock.getElapsedTime().asSeconds());
            int minutes = timeElapsed / 60;
            int seconds = timeElapsed % 60;
            timeText.setString("Tiempo " + to_string(minutes) + " minutos " +
                (seconds < 10 ? "0" : "") + to_string(seconds) + " segundos") ;

            // Dibujar todos los textos
            window.draw(scoreText);
            window.draw(visitedText);
            window.draw(energyText);
            window.draw(positionText);
            window.draw(pathText);
            window.draw(timeText);
            window.draw(instructionsText);
            window.draw(automaticMode);

            // Información adicional en la esquina superior derecha
            Text statusText;
            statusText.setFont(font);
            statusText.setCharacterSize(12);
            statusText.setFillColor(Color(200, 200, 255));
            statusText.setPosition(WINDOW_WIDTH - 200, 10);

            string statusInfo = "Estado del juego \n";
            statusInfo += "Muro roto " + string(wallBreakUsed ? "SI" : "NO") + "\n";
            statusInfo += "Nivel agua " + to_string(static_cast<int>(waterClock.getElapsedTime().asSeconds() / WATER_STEP_INTERVAL)) + "\n";

            // Calcular eficiencia
            float efficiency = visited.size() > 0 ? (float)score / visited.size() : 0;
            statusInfo += "Eficiencia " + to_string(static_cast<int>(efficiency)) + " pts por celda";

            statusText.setString(statusInfo);
            window.draw(statusText);
        }

        window.display();
    }

    return 0;
}