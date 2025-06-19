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
#include "AStarPathfinder.h"
#include "UIHelper.h"


using namespace sf;
using namespace std;


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
	ifstream check("fonts/ARCADECLASSIC.TTF");
	if (!check.is_open()) {
		cerr << " No se puede abrir fonts/ARCADECLASSIC.TTF desde C++" << endl;
		return -1;
	}

	sf::Font font;
	if (!font.loadFromFile("fonts/ARCADECLASSIC.TTF")) {
		cerr << "Error cargando fuente" << endl;
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
					// Verificar que haya un intento de movimiento real
					if (newRow != player.row || newCol != player.col) {
						pair<int, int> target = { newRow, newCol };

						// Verificar que la celda es válida para moverse
						if (newRow >= 0 && newRow < GRID_ROWS &&
							newCol >= 0 && newCol < GRID_COLS &&
							!grid[newRow][newCol].isWall &&
							!grid[newRow][newCol].isFlooded) {

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
			}
			else if (event.key.code == Keyboard::M) {
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
				(seconds < 10 ? "0" : "") + to_string(seconds) + " segundos");

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