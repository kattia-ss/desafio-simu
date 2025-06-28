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
#include <cstdlib>  // rand


// #include "FileSelector.h"

// delcaración de funciones auxiliares
bool loadMapWithValidation(const string& filename, vector<vector<HexagonCell>>& grid);

using namespace sf;
using namespace std;

//burbujas
vector<CircleShape> bubbles;
vector<float> bubbleSpeeds;
Texture coralTexture;
Sprite coral;
bool coralLoaded = false;

//pez prron
vector<CircleShape> fish;
vector<float> fishSpeed;

//alguien dijo PEZ
vector<Sprite> fishSprites;
vector<float> fishSpeeds;
Texture fishTexture;
bool fishTextureLoaded = false;

//corales para el mapa
Sprite coralLeft, coralRight;
bool coralsDecorReady = false;

//Fondo para el mapa
VertexArray backgroundQuad(Quads, 4);

//para recoger un item
struct Particle {
	Vector2f pos, vel;
	float life;
};
vector<Particle> particles;


//efecto al caminar
struct Ripple {
	Vector2f position;
	float radius;
	float opacity;
};
vector<Ripple> ripples;

Texture wallTexture;
bool wallTextureLoaded = false;




void drawIntroDecorations(RenderWindow& window, float windowWidth, float windowHeight) {
	// Animar y dibujar burbujas
	for (size_t i = 0; i < bubbles.size(); ++i) {
		Vector2f pos = bubbles[i].getPosition();
		pos.y -= bubbleSpeeds[i];
		if (pos.y < -10) {
			pos.y = windowHeight + rand() % 100;
			pos.x = rand() % static_cast<int>(windowWidth);
		}
		bubbles[i].setPosition(pos);
		window.draw(bubbles[i]);
	}

	// Dibujar coral una vez
	if (coralLoaded) {
		window.draw(coral);

		Sprite coral2 = coral;
		coral2.setPosition(windowWidth - 150, windowHeight - 130);
		coral2.setScale(0.4f, 0.4f);
		window.draw(coral2);
	}

	//peces prrones (es un triangulo)
	for (size_t i = 0; i < fish.size(); ++i) {
		Vector2f pos = fish[i].getPosition();
		pos.x += fishSpeed[i]; // nadan hacia la derecha
		if (pos.x > windowWidth + 50) {
			pos.x = -100;
			pos.y = 100 + rand() % static_cast<int>(windowHeight - 200);
		}
		fish[i].setPosition(pos);
		window.draw(fish[i]);
	}


	//pez bonito obvio
	if (fishTextureLoaded) {
		for (size_t i = 0; i < fishSprites.size(); ++i) {
			Vector2f pos = fishSprites[i].getPosition();
			pos.x += fishSpeeds[i];
			if (pos.x > windowWidth + 50) {
				pos.x = -100;
				pos.y = 100 + rand() % static_cast<int>(windowHeight - 200);
			}
			fishSprites[i].setPosition(pos);
			window.draw(fishSprites[i]);
		}
	}


}

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

	backgroundQuad[0].position = Vector2f(0, 0);
	backgroundQuad[1].position = Vector2f(windowWidth, 0);
	backgroundQuad[2].position = Vector2f(windowWidth, windowHeight);
	backgroundQuad[3].position = Vector2f(0, windowHeight);

	// Colores del gradiente (top ? bottom)
	backgroundQuad[0].color = Color(10, 20, 40);   // azul profundo arriba izquierda
	backgroundQuad[1].color = Color(10, 20, 40);   // azul profundo arriba derecha
	backgroundQuad[2].color = Color(20, 180, 140); // verde esmeralda abajo derecha
	backgroundQuad[3].color = Color(20, 180, 140); // verde esmeralda abajo izquierda


	View view(FloatRect(0, 0, windowWidth, windowHeight));
	view.setCenter(windowWidth / 2, windowHeight / 2);
	window.setView(view);

	//esto es despues de crear la ventana
	


	// Inicializar burbujas
	for (int i = 0; i < 30; ++i) {
		CircleShape bubble(rand() % 5 + 2);
		bubble.setFillColor(Color(200, 255, 255, 100)); // semitransparente
		bubble.setPosition(rand() % (int)windowWidth, windowHeight + rand() % 400);
		bubbles.push_back(bubble);
		bubbleSpeeds.push_back(0.3f + static_cast<float>(rand() % 100) / 300.f);
	}
	if (coralTexture.loadFromFile("assets/coral.png")) {
		coral.setTexture(coralTexture);
		coral.setScale(0.5f, 0.5f); // ajustá según tamaño original
		coral.setPosition(60, windowHeight - 140); // esquina inferior izquierda
		coral.setColor(Color::White); 
		coralLoaded = true;
	}
	if (coralLoaded) {
		coralLeft = coral;
		coralLeft.setScale(0.5f, 0.5f);
		coralLeft.setPosition(50, windowHeight - 150);

		coralRight = coral;
		coralRight.setScale(-0.5f, 0.5f); // reflejado horizontalmente
		coralRight.setPosition(windowWidth - 150, windowHeight - 150);

		coralsDecorReady = true;
	}
	else {
		cerr << "No se pudo cargar assets/coral.png\n";
	}

	if (fishTexture.loadFromFile("assets/fish1.png")) {
		fishTextureLoaded = true;

		// Crear múltiples peces
		for (int i = 0; i < 5; ++i) {
			Sprite fish(fishTexture);
			fish.setScale(0.2f + static_cast<float>(rand() % 10) / 50.f, 0.2f); // escalado variado
			float y = 100 + rand() % static_cast<int>(windowHeight - 200);
			fish.setPosition(-rand() % 300, y);
			fishSprites.push_back(fish);
			fishSpeeds.push_back(0.4f + static_cast<float>(rand() % 100) / 200.f);
		}
	}
	else {
		cerr << "? No se pudo cargar assets/fish1.png\n";
	}

	



	//pez
	// Crear peces simples (forma triangular)
	for (int i = 0; i < 5; ++i) {
		CircleShape f(6, 3); // forma de triángulo
		f.setFillColor(Color(100 + rand() % 156, 100 + rand() % 156, rand() % 100)); //color arcoirs
		f.setScale(1.0f + rand() % 3 / 5.f, 1.0f);
		float y = 100 + rand() % static_cast<int>(windowHeight - 200);
		f.setPosition(-rand() % 300, y);
		f.setRotation(90); // apuntar hacia la derecha
		fish.push_back(f);
		fishSpeed.push_back(0.3f + static_cast<float>(rand() % 100) / 300.f);
	}



	
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


	bool esperandoInicio = true;
	// Pantalla de bienvenida
	Text introTitle, introText, introHint;
	
	

	// Título en turquesa brillante
	introTitle.setFont(font);
	introTitle.setCharacterSize(28);
	introTitle.setFillColor(Color(64, 224, 208)); // Turquesa
	introTitle.setString("TEMPLO ABISAL");
	introTitle.setPosition(windowWidth / 2 - introTitle.getLocalBounds().width / 2, 80);

	// Texto blanco con matices oceánicos
	introText.setFont(font);
	introText.setCharacterSize(16);
	introText.setFillColor(Color(200, 255, 255));  // Azul claro
	introText.setLineSpacing(1.5f);
	introText.setString(
		"Desciende al templo sumergido, evita la inundación\n"
		"y encuentra la salida antes de que el abismo te consuma.\n\n"
		"Controles:\n"
		"W/E = Arriba Diagonal | A/D = Izquierda/Derecha\n"
		"Z/X = Abajo Diagonal | O/L = Zoom In/Out\n"
		"F = Ruta A* | R = Romper muro | M = Automático | ESC = Salir"
	);
	introText.setPosition(windowWidth / 2 - introText.getLocalBounds().width / 2, 160);

	// Texto inferior
	introHint.setFont(font);
	introHint.setCharacterSize(14);
	introHint.setFillColor(Color(180, 220, 250));
	introHint.setString("Haz CLICK y luego presiona ESPACIO para sumergirte...");
	introHint.setPosition(windowWidth / 2 - introHint.getLocalBounds().width / 2, windowHeight - 100);

	while (esperandoInicio) {
		Event e;
		while (window.pollEvent(e)) {
			if (e.type == Event::Closed)
				window.close();
			if (e.type == Event::KeyPressed && e.key.code == Keyboard::Space)
				esperandoInicio = false;
		}

		// Redibujar cada frame
		window.clear(Color(10, 20, 40));
		drawIntroDecorations(window, windowWidth, windowHeight);
		window.draw(introTitle);
		window.draw(introText);
		window.draw(introHint);
		window.display();
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
			system("pause"); // <- agrega esto para ver el error en consola
			return -1;
		}
	}

	if (grid.empty() || grid[0].empty()) {
		cerr << "Error: el mapa se cargó pero la grilla está vacía o corrupta.\n";
		system("pause");
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
				Vector2f ripplePos = grid[player.row][player.col].getPosition();
				ripples.push_back({ ripplePos, 5.f, 255.f });
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

			bool esCeldaNormal = !grid[r][c].isWall && !grid[r][c].isStart &&
				!grid[r][c].isGoal && !grid[r][c].isItem;

			if (esCeldaNormal) {
				int minH = 0, maxH = 60;
				float ratio = static_cast<float>(grid[r][c].height - minH) / (maxH - minH);
				ratio = std::clamp(ratio, 0.f, 1.f);
				ratio = pow(ratio, 0.85f);

				int rColor = static_cast<int>(20 + 40 * ratio);
				int gColor = static_cast<int>(40 + 190 * ratio);
				int bColor = static_cast<int>(90 - 70 * ratio);

				Color cellColor(rColor, gColor, bColor);
				grid[r][c].setFillColor(cellColor);
			}

			originalColors[r][c] = grid[r][c].getFillColor();

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
								Vector2f itemPos = grid[newRow][newCol].getPosition();
								for (int i = 0; i < 10; ++i) {
									float angle = (rand() % 360) * 3.14159f / 180.f;
									float speed = 1.5f + rand() % 30 / 10.f;
									particles.push_back({
										itemPos,
										Vector2f(cos(angle) * speed, sin(angle) * speed),
										255.f
										});
								}

								cout << "?? ¡ITEM RECOGIDO! +100 puntos. Puntuacion: " << score << endl;
							}

							player.row = newRow;
							player.col = newCol;
							Vector2f ripplePos = grid[player.row][player.col].getPosition();
							ripples.push_back({ ripplePos, 5.f, 255.f });

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
						Vector2f breakPos = grid[nr][nc].getPosition();
						for (int i = 0; i < 15; ++i) {
							float angle = (rand() % 360) * 3.14159f / 180.f;
							float speed = 1.0f + rand() % 40 / 10.f;
							particles.push_back({
								breakPos,
								Vector2f(cos(angle) * speed, sin(angle) * speed),
								255.f
								});
						}
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

		window.setView(window.getDefaultView()); // UI y decoraciones fijas a pantalla
		//para que tenga algo bonito
		window.clear(); // sin color
		window.draw(backgroundQuad);

		// Dibujar burbujas animadas
		for (size_t i = 0; i < bubbles.size(); ++i) {
			Vector2f pos = bubbles[i].getPosition();
			pos.y -= bubbleSpeeds[i];
			if (pos.y < -10) {
				pos.y = window.getSize().y + rand() % 100;
				pos.x = rand() % (int)window.getSize().x;
			}
			bubbles[i].setPosition(pos);
			window.draw(bubbles[i]);
		}

		//poner corales
		if (coralsDecorReady) {
			window.draw(coralLeft);
			window.draw(coralRight);
		}

		//poner peces bonitos
		if (fishTextureLoaded) {
			for (size_t i = 0; i < fishSprites.size(); ++i) {
				Vector2f pos = fishSprites[i].getPosition();
				pos.x += fishSpeeds[i];
				if (pos.x > windowWidth + 50) {
					pos.x = -100;
					pos.y = 100 + rand() % static_cast<int>(windowHeight - 200);
				}
				fishSprites[i].setPosition(pos);
				window.draw(fishSprites[i]);
			}
		}

		window.setView(view); // volver a la vista con zoom
		


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
				pathCell.setFillColor(Color(0, 200, 100, 180)); // Verde semitransparente
				window.draw(pathCell);
			}
		}

		// Dibujar el jugador
		CircleShape highlight(HEX_RADIUS / 2.f, 6);
		highlight.setFillColor(Color(156, 39, 176)); // púrpura oscuro vibrante
		highlight.setOrigin(highlight.getRadius(), highlight.getRadius());
		highlight.setPosition(grid[player.row][player.col].getPosition());
		window.draw(highlight);

		// Dibujar ondas **después**
		for (auto& r : ripples) {
			CircleShape ring(r.radius);
			ring.setOrigin(r.radius, r.radius);
			ring.setPosition(r.position);
			ring.setFillColor(Color::Transparent);
			ring.setOutlineThickness(2);
			ring.setOutlineColor(Color(0, 255, 255, static_cast<Uint8>(r.opacity)));
			window.draw(ring);

			r.radius += 1.5f;
			r.opacity -= 4.f;
		}
		ripples.erase(remove_if(ripples.begin(), ripples.end(),
			[](const Ripple& r) { return r.opacity <= 0.f; }), ripples.end());

		for (auto& p : particles) {
			CircleShape spark(2);
			spark.setPosition(p.pos);
			spark.setFillColor(Color(255, 255, 0, static_cast<Uint8>(p.life)));
			window.draw(spark);

			p.pos += p.vel;
			p.life -= 3.f;
		}
		particles.erase(remove_if(particles.begin(), particles.end(),
			[](const Particle& p) { return p.life <= 0.f; }), particles.end());

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

