#include "FileSelector.h"
#include <fstream>
#include <algorithm>
#include <iostream>
#include "json.hpp"

#ifdef _WIN32
#include <filesystem>
namespace fs = std::filesystem;
#else
#include <dirent.h>
#include <sys/stat.h>
#endif

using json = nlohmann::json;
using namespace std;
using namespace sf;

FileSelector::FileSelector() : selectedIndex(0), showSelector(false) {
    scanJsonFiles();
}

void FileSelector::scanJsonFiles() {
    jsonFiles.clear();

    // Buscar archivos JSON en el directorio actual
    try {
#ifdef _WIN32
        // Usar std::filesystem en Windows
        for (const auto& entry : fs::directory_iterator(".")) {
            if (entry.is_regular_file() && entry.path().extension() == ".json") {
                jsonFiles.push_back(entry.path().filename().string());
            }
        }
#else
        // Usar dirent.h para sistemas Unix/Linux
        DIR* dir = opendir(".");
        if (dir != nullptr) {
            struct dirent* entry;
            while ((entry = readdir(dir)) != nullptr) {
                string filename = entry->d_name;
                if (filename.length() > 5 &&
                    filename.substr(filename.length() - 5) == ".json") {
                    // Verificar que es un archivo regular
                    struct stat statbuf;
                    if (stat(filename.c_str(), &statbuf) == 0 && S_ISREG(statbuf.st_mode)) {
                        jsonFiles.push_back(filename);
                    }
                }
            }
            closedir(dir);
        }
#endif
    }
    catch (const exception& e) {
        cerr << "Error escaneando archivos JSON: " << e.what() << endl;
    }

    // Ordenar alfabéticamente
    sort(jsonFiles.begin(), jsonFiles.end());

    // Agregar opción para mapa por defecto al inicio
    jsonFiles.insert(jsonFiles.begin(), "mapa2.json (default)");

    // Resetear índice si es necesario
    if (selectedIndex >= jsonFiles.size()) {
        selectedIndex = 0;
    }
}

void FileSelector::handleInput(const Event& event) {
    if (!showSelector) return;

    if (event.type == Event::KeyPressed) {
        switch (event.key.code) {
        case Keyboard::Up:
            if (selectedIndex > 0) selectedIndex--;
            break;
        case Keyboard::Down:
            if (selectedIndex < jsonFiles.size() - 1) selectedIndex++;
            break;
        case Keyboard::Enter:
            if (!jsonFiles.empty()) {
                selectedFile = jsonFiles[selectedIndex];
                // Remover el texto "(default)" si está presente
                if (selectedFile.find("(default)") != string::npos) {
                    selectedFile = "mapa2.json";
                }
                showSelector = false;
            }
            break;
        case Keyboard::Escape:
            showSelector = false;
            selectedFile = "mapa2.json"; // Volver al mapa por defecto
            break;
        case Keyboard::F5:
            scanJsonFiles(); // Reescanear archivos
            break;
        }
    }
}

void FileSelector::draw(RenderWindow& window, const Font& font) {
    if (!showSelector) return;

    // Fondo semitransparente
    RectangleShape background(Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
    background.setFillColor(Color(0, 0, 0, 200));
    window.draw(background);

    // Panel del selector
    RectangleShape panel(Vector2f(600, 400));
    panel.setPosition((WINDOW_WIDTH - 600) / 2.f, (WINDOW_HEIGHT - 400) / 2.f);
    panel.setFillColor(Color(40, 40, 40));
    panel.setOutlineColor(Color::White);
    panel.setOutlineThickness(2);
    window.draw(panel);

    // Título
    Text title;
    title.setFont(font);
    title.setString("SELECCIONAR MAPA JSON");
    title.setCharacterSize(24);
    title.setFillColor(Color::Yellow);
    title.setOrigin(title.getLocalBounds().width / 2, 0);
    title.setPosition(WINDOW_WIDTH / 2.f, panel.getPosition().y + 20);
    window.draw(title);

    // Lista de archivos
    float startY = panel.getPosition().y + 80;
    float itemHeight = 25;

    for (size_t i = 0; i < jsonFiles.size(); ++i) {
        Text fileText;
        fileText.setFont(font);
        fileText.setString(jsonFiles[i]);
        fileText.setCharacterSize(16);

        // Resaltar archivo seleccionado
        if (i == selectedIndex) {
            RectangleShape highlight(Vector2f(560, itemHeight));
            highlight.setPosition(panel.getPosition().x + 20, startY + i * itemHeight - 2);
            highlight.setFillColor(Color(100, 100, 255, 100));
            window.draw(highlight);
            fileText.setFillColor(Color::Yellow);
        }
        else {
            fileText.setFillColor(Color::White);
        }

        fileText.setPosition(panel.getPosition().x + 30, startY + i * itemHeight);
        window.draw(fileText);
    }

    // Instrucciones
    Text instructions;
    instructions.setFont(font);
    instructions.setString(
        "Controles:\n"
        "?/? = Navegar | ENTER = Seleccionar\n"
        "ESC = Cancelar | F5 = Actualizar lista"
    );
    instructions.setCharacterSize(12);
    instructions.setFillColor(Color(200, 200, 200));
    instructions.setPosition(panel.getPosition().x + 20, panel.getPosition().y + 320);
    window.draw(instructions);

    // Estado
    Text status;
    status.setFont(font);
    status.setString("Archivos encontrados: " + to_string(jsonFiles.size()));
    status.setCharacterSize(12);
    status.setFillColor(Color(150, 255, 150));
    status.setPosition(panel.getPosition().x + 20, panel.getPosition().y + 370);
    window.draw(status);
}

string FileSelector::getSelectedFile() const {
    return selectedFile;
}

void FileSelector::show() {
    showSelector = true;
    scanJsonFiles(); // Actualizar lista al mostrar
}

void FileSelector::hide() {
    showSelector = false;
}

bool FileSelector::isVisible() const {
    return showSelector;
}

// Validar que el archivo JSON tiene la estructura correcta
bool FileSelector::validateJsonStructure(const string& filename) {
    try {
        ifstream file(filename);
        if (!file.is_open()) return false;

        json j;
        file >> j;

        // Verificar campos requeridos
        if (!j.contains("rows") || !j.contains("cols") ||
            !j.contains("grid") || !j.contains("heightMap")) {
            return false;
        }

        int rows = j["rows"];
        int cols = j["cols"];

        // Verificar que las dimensiones sean válidas
        if (rows <= 0 || cols <= 0 || rows > 50 || cols > 50) {
            return false;
        }

        // Verificar que grid y heightMap tengan las dimensiones correctas
        if (j["grid"].size() != rows || j["heightMap"].size() != rows) {
            return false;
        }

        for (int r = 0; r < rows; ++r) {
            if (j["grid"][r].size() != cols || j["heightMap"][r].size() != cols) {
                return false;
            }
        }

        // Verificar que haya al menos un punto de inicio y un objetivo
        bool hasStart = false, hasGoal = false;
        for (int r = 0; r < rows; ++r) {
            for (int c = 0; c < cols; ++c) {
                string cell = j["grid"][r][c];
                if (!cell.empty()) {
                    char symbol = cell[0];
                    if (symbol == 'S') hasStart = true;
                    if (symbol == 'G') hasGoal = true;
                }
            }
        }

        return hasStart && hasGoal;

    }
    catch (const exception& e) {
        cerr << "Error validando JSON: " << e.what() << endl;
        return false;
    }
}