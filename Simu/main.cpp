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
#include "loadMapSelector.h"


// #include "FileSelector.h"

// delcaración de funciones auxiliares
bool loadMapWithValidation(const string& filename, vector<vector<HexagonCell>>& grid);

using namespace sf;
using namespace std;



int main() {

	//Para resolver automaticamente
	bool showStartScreen = true;
	bool autoMoveEnabled = false;
	size_t autoMoveIndex = 1;  // comienza desde 1 porque 0 es la posición actual del jugador
	Clock autoMoveClock;
	const float AUTO_MOVE_INTERVAL = 0.3f; // segundos entre cada paso

	// Seleccionar mapa
	string selectedMapFile;
	cout << "==============================" << endl;
	cout << " Ingrese la ruta del mapa:" << endl;
	cout << " (o presione ENTER para usar 'mapa2.json')" << endl;
	cout << "==============================" << endl;
	getline(cin, selectedMapFile);

	if (selectedMapFile.empty()) {
		selectedMapFile = "mapa2.json";
		cout << "Usando mapa por defecto: " << selectedMapFile << endl;
	}
	else {
		cout << "Cargando mapa: " << selectedMapFile << endl;
	}

	RenderWindow window(VideoMode::getDesktopMode(), "Templo", Style::Fullscreen);
	window.setFramerateLimit(60);

	float windowWidth = window.getSize().x;
	float windowHeight = window.getSize().y;

	View view(FloatRect(0, 0, windowWidth, windowHeight));
	view.setCenter(windowWidth / 2, windowHeight / 2);
	window.setView(view);

	
	// HUD retro visual.......................................................
	RectangleShape hudPanel;
	hudPanel.setSize(Vector2f(400, 200));  // Tamaño del HUD
	hudPanel.setPosition(20, 20);           // Posición en pantalla
	hudPanel.setFillColor(Color(0, 0, 0, 160));     // Fondo negro semitransparente
	hudPanel.setOutlineColor(Color::White);        // Borde blanco
	hudPanel.setOutlineThickness(1.5f);            // Grosor del borde


	RectangleShape statusPanel;
	statusPanel.setSize(Vector2f(250, 100));
	statusPanel.setPosition(windowWidth - statusPanel.getSize().x - 20, 20);
	statusPanel.setFillColor(Color(0, 0, 0, 160));    // Fondo semitransparente
	statusPanel.setOutlineColor(Color::White);
	statusPanel.setOutlineThickness(1.5f);

	//........................................................................


	RectangleShape energyBarBack(Vector2f(120, 12));
	energyBarBack.setPosition(hudPanel.getPosition().x + 10, hudPanel.getPosition().y + 170);
	energyBarBack.setFillColor(Color(50, 50, 50));

	RectangleShape energyBarFill(Vector2f(0, 12));
	energyBarFill.setPosition(hudPanel.getPosition().x + 10, hudPanel.getPosition().y + 170);
	energyBarFill.setFillColor(Color(0, 200, 255));


	// Intentar cargar la fuente 

	bool fontLoaded = false;
	ifstream check("fonts/PRESSSTART2P.TTF");
	if (!check.is_open()) {
		cerr << " No se puede abrir fonts/PRESSSTART2P.TTF desde C++" << endl;
		return -1;
	}

	Font font;
	if (!font.loadFromFile("fonts/PRESSSTART2P.TTF")) {
		cerr << "Error cargando fuente" << endl;
		return -1;
	}

	// Pantalla de bienvenida
	Text introTitle, introText, introHint;
	introTitle.setFont(font);
	introTitle.setCharacterSize(24);
	introTitle.setFillColor(Color::Yellow);
	introTitle.setString("TEMPLO - Aventura Hexagonal");
	introTitle.setPosition(windowWidth / 2 - introTitle.getLocalBounds().width / 2, 100);

	introText.setFont(font);
	introText.setCharacterSize(16);
	introText.setFillColor(Color::White);
	introText.setLineSpacing(1.5f);
	introText.setString(
		"Explora el templo, evita el agua y alcanza el objetivo final.\n\n"
		"Controles:\n"
		"W/E = Arriba Diagonal\nA/D = Izquierda / Derecha\nZ/X = Abajo Diagonal\n\n"
		"F = Ruta A* | R = Romper muro | M = Modo automático | ESC = Salir"
	);
	introText.setPosition(windowWidth / 2 - introText.getLocalBounds().width / 2, 200);

	introHint.setFont(font);
	introHint.setCharacterSize(14);
	introHint.setFillColor(Color(180, 180, 180));
	introHint.setString("Presiona ESPACIO para comenzar...");
	introHint.setPosition(windowWidth / 2 - introHint.getLocalBounds().width / 2, 500);

	window.clear(Color::Black);
	window.draw(introTitle);
	window.draw(introText);
	window.draw(introHint);
	window.display();

	// Esperar ESPACIO para comenzar
	bool esperandoInicio = true;
	while (esperandoInicio) {
		Event e;
		while (window.pollEvent(e)) {
			if (e.type == Event::Closed)
				window.close();
			if (e.type == Event::KeyPressed && e.key.code == Keyboard::Space)
				esperandoInicio = false;
		}
	}

	// Opcional: limpiar terminal al comenzar
#ifdef _WIN32
	system("cls");
#else
	system("clear");
#endif



	

	// Cargar mapa con validación
	vector<vector<HexagonCell>> grid;
	if (!loadMapWithValidation(selectedMapFile, grid)) {
		cout << "Intentando cargar mapa por defecto..." << endl;
		if (!loadMapWithValidation("mapa2.json", grid)) {
			cerr << "Error : No se pudo cargar ningún mapa válido" << endl;
			return -1;
		}
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
		cerr << "START cell not found.\n";
		return -1;
	}

	float totalGridWidth = (GRID_COLS - 1) * (HEX_APOTHEM * 2 + HEX_SPACING) + HEX_APOTHEM * 2;
	float totalGridHeight = (GRID_ROWS - 1) * (HEX_RADIUS * 1.5f + HEX_SPACING) + HEX_RADIUS * 2;
	float offsetX = (windowWidth - totalGridWidth) / 2.f;
	float offsetY = (windowHeight - totalGridHeight) / 2.f;

	// Guardar colores originales para restaurar después
	vector<vector<Color>> originalColors(GRID_ROWS, vector<Color>(GRID_COLS));
	for (int r = 0; r < GRID_ROWS; ++r) {
		for (int c = 0; c < GRID_COLS; ++c) {
			grid[r][c].setScreenPosition(c, r, offsetX, offsetY);

			if (!grid[r][c].isWall && !grid[r][c].isStart && !grid[r][c].isGoal && !grid[r][c].isItem) {
				int minH = 0, maxH = 60;
				float ratio = static_cast<float>(grid[r][c].height - minH) / (maxH - minH);
				ratio = std::clamp(ratio, 0.f, 1.f);

				// Correccion para transicion mas suave en medio
				ratio = pow(ratio, 0.85f);

				

				// Color: azul marino a verde esmeralda
				int rColor = static_cast<int>(20 + 40 * ratio);      // de 20 a 60
				int gColor = static_cast<int>(40 + 190 * ratio);     // de 40 a 230 a mucho verde arriba
				int bColor = static_cast<int>(90 - 70 * ratio);      // de 90 a 20 a azul se reduce

				Color cellColor(rColor, gColor, bColor);
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




	// Crear textos solo si la fuente se cargó correctamente
	Text scoreText, visitedText, energyText, positionText, pathText, timeText, instructionsText, automaticMode;
	Clock gameClock; // Para mostrar tiempo transcurrido

	fontLoaded = true;

	if (fontLoaded) {
		float hudX = hudPanel.getPosition().x + 10;
		float hudY = hudPanel.getPosition().y + 10;

		// Posicionar barra después del último texto
		float energyBarY = hudY + 130 + 30;  // 130 por los textos, +30 de separación

		// Puntuación
		scoreText.setFont(font);
		scoreText.setCharacterSize(18);
		scoreText.setFillColor(Color::White);
		scoreText.setPosition(hudX, hudY);

		// Barra de energía
		energyBarBack.setPosition(hudX, energyBarY);
energyBarFill.setPosition(hudX, energyBarY);

		// Texto de energía
		energyText.setFont(font);
		energyText.setCharacterSize(12);
		energyText.setFillColor(Color(0, 200, 255));
		energyText.setPosition(hudX, hudY + 39);

		// Celdas visitadas
		visitedText.setFont(font);
		visitedText.setCharacterSize(12);
		visitedText.setFillColor(Color(200, 200, 200));
		visitedText.setPosition(hudX, hudY + 58);

		// Posición del jugador
		positionText.setFont(font);
		positionText.setCharacterSize(12);
		positionText.setFillColor(Color(255, 255, 0));
		positionText.setPosition(hudX, hudY + 76);

		// Información del camino
		pathText.setFont(font);
		pathText.setCharacterSize(12);
		pathText.setFillColor(Color(255, 165, 0));
		pathText.setPosition(hudX, hudY + 94);

		// Tiempo transcurrido
		timeText.setFont(font);
		timeText.setCharacterSize(12);
		timeText.setFillColor(Color(150, 255, 150));
		timeText.setPosition(hudX, hudY + 112);

		// Modo automático
		automaticMode.setFont(font);
		automaticMode.setCharacterSize(12);
		automaticMode.setFillColor(Color::White);
		automaticMode.setPosition(hudX, hudY + 130);


		// Instrucciones (abajo)
		instructionsText.setFont(font);
		instructionsText.setCharacterSize(14);
		instructionsText.setFillColor(Color(180, 180, 180));
		instructionsText.setLineSpacing(1.3f);
		instructionsText.setPosition(20, windowHeight - 70);
		instructionsText.setString(
			"Controles: W/E = Arriba diag | A/D = Izq/Der | Z/X = Abajo diag\n"
			"F = Ruta A* | R = Romper muro | M = Auto | ESC = Salir"
		);
	}

	waterClock.restart();
	gameClock.restart();


	// inicio del programa principal
	while (window.isOpen()) {
		window.setView(view);
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

				if (event.key.code == Keyboard::O) {
					view.zoom(0.9f); // Zoom in
					window.setView(view);
				}
				else if (event.key.code == Keyboard::L) {
					view.zoom(1.1f); // Zoom out
					window.setView(view);
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
								window.setView(window.getDefaultView());
								showEndScreen(window, "Felicidades ganaste", font);
							}
						}
					}
				}

				if (event.key.code == Keyboard::R && energy == MAX_ENERGY && !wallBreakUsed) {

					int nr = player.row + lastMoveDir.first;
					int nc = player.col + lastMoveDir.second;

					if (nr >= 0 && nr < GRID_ROWS && nc >= 0 && nc < GRID_COLS && grid[nr][nc].isWall) {
						wallBreakUsed = true;
						grid[nr][nc].isWall = false;
						grid[nr][nc].setFillColor(Color::White);

						energy = 0;
						// Establecer altura alta para que se inunde
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
						cout << "?? Muro roto con altura igual a la meta: " << goalHeight << endl;
					}
				}
				//para resolver automatico cuando presione la letra M 
				else if (event.key.code == Keyboard::M) {
					if (autoMoveEnabled) {
						// Si ya está activo, desactivar modo automático
						autoMoveEnabled = false;
						cout << "?? Modo automático DESACTIVADO\n";
					}
					else {
						// Calcular nueva ruta y activar
						for (int r = 0; r < GRID_ROWS; ++r) {
							for (int c = 0; c < GRID_COLS; ++c) {
								if (grid[r][c].isGoal) {
									currentPath = findPathAStar(grid, player.row, player.col, r, c);
									autoMoveIndex = 1;
									autoMoveEnabled = !currentPath.empty();
									if (autoMoveEnabled) {
										cout << "?? Modo automático ACTIVADO\n";
									}
									else {
										cout << "?? No se pudo calcular ruta automática\n";
									}
									break;
								}
							}
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
						grid[r][c].setFillColor(Color(0, 204, 255));
					}
					else if (grid[r][c].isGoal && waterLevel > maxGoalHeight + 2) {
						grid[r][c].isFlooded = true;
						grid[r][c].setFillColor(Color(0, 204, 255));
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
			window.setView(window.getDefaultView());
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

					// Recalcular ruta automáticamente si el paso está bloqueado
					cout << "?? Ruta bloqueada por agua. Recalculando..." << endl;

					currentPath = findPathAStar(grid, player.row, player.col,
						// encontrar la meta nuevamente
						[&]() -> pair<int, int> {
							for (int r = 0; r < GRID_ROWS; ++r)
								for (int c = 0; c < GRID_COLS; ++c)
									if (grid[r][c].isGoal)
										return { r, c };
							return { -1, -1 };
						}().first,
							[&]() -> pair<int, int> {
							for (int r = 0; r < GRID_ROWS; ++r)
								for (int c = 0; c < GRID_COLS; ++c)
									if (grid[r][c].isGoal)
										return { r, c };
							return { -1, -1 };
							}().second
								);

					autoMoveIndex = 1;
					autoMoveEnabled = !currentPath.empty();

					if (!autoMoveEnabled) {
						cout << "? No hay ruta disponible, modo automático cancelado." << endl;
					}
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
		highlight.setFillColor(Color(156, 39, 176)); // púrpura oscuro vibrante
		highlight.setOrigin(highlight.getRadius(), highlight.getRadius());
		highlight.setPosition(grid[player.row][player.col].getPosition());
		window.draw(highlight);

		// Dibujar la barra de energía
		energyBarFill.setSize(Vector2f(120 * (static_cast<float>(energy) / MAX_ENERGY), 12));
	

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

			// Información adicional en la esquina superior derecha
			Text statusText;
			statusText.setFont(font);
			statusText.setCharacterSize(12);
			statusText.setFillColor(Color(200, 200, 255));
			statusText.setPosition(statusPanel.getPosition().x + 10, statusPanel.getPosition().y + 10);

			// Contenido del estado
			string statusInfo = "Estado del juego\n";
			statusInfo += "Muro roto " + string(wallBreakUsed ? "SI" : "NO") + "\n";
			statusInfo += "Nivel agua " + to_string(static_cast<int>(waterClock.getElapsedTime().asSeconds() / WATER_STEP_INTERVAL)) + "\n";

			// Calcular eficiencia
			float efficiency = visited.size() > 0 ? (float)score / visited.size() : 0;
			statusInfo += "Eficiencia:\n" + to_string((int)efficiency) + " pts/celda";

			statusText.setString(statusInfo);


			window.setView(window.getDefaultView());
			//barra de energia
			window.draw(energyBarBack);
			window.draw(energyBarFill);

			// Dibujar HUD principal
			window.draw(hudPanel);
			window.draw(scoreText);
			window.draw(visitedText);
			window.draw(energyText);
			window.draw(positionText);
			window.draw(pathText);
			window.draw(timeText);
			window.draw(automaticMode);

			// Dibujar HUD de estado
			window.draw(statusPanel);
			window.draw(statusText);

			// Instrucciones (abajo)
			window.draw(instructionsText);
		}

		window.display();
	}

	return 0;
}

