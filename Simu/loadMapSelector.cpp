#include "json.hpp" 
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "MapLoader.h"
#include "HexagonCell.h" 

using namespace std;
using json = nlohmann::json;

bool loadMapWithValidation(const string& filename, vector<vector<HexagonCell>>& grid) {
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "Error: No se pudo abrir el archivo " << filename << endl;
        return false;
    }

    try {
        json j;
        file >> j;

        if (!j.contains("rows") || !j.contains("cols") || !j.contains("grid") || !j.contains("heightMap")) {
            cout << "Error: El archivo no contiene todos los campos requeridos." << endl;
            return false;
        }

        int rows = j["rows"];
        int cols = j["cols"];

        if (rows <= 0 || cols <= 0 || j["grid"].size() != rows || j["heightMap"].size() != rows) {
            cout << "Error: Dimensiones inválidas en el archivo JSON." << endl;
            return false;
        }

        for (int r = 0; r < rows; ++r) {
            if (j["grid"][r].size() != cols || j["heightMap"][r].size() != cols) {
                cout << "Error: Inconsistencia en el tamaño de filas/columnas." << endl;
                return false;
            }
        }

        // Contadores para validar que solo haya una 'S' y una 'G'
        int startCount = 0;
        int goalCount = 0;

        for (int r = 0; r < rows; ++r) {
            for (int c = 0; c < cols; ++c) {
                string cell = j["grid"][r][c];
                if (!cell.empty()) {
                    char symbol = cell[0];
                    if (symbol == 'S') startCount++;
                    if (symbol == 'G') goalCount++;
                }
            }
        }

        if (startCount != 1 || goalCount != 1) {
            cout << "Error: El mapa debe tener exactamente UNA celda 'S' y UNA celda 'G'." << endl;
            cout << "Encontradas: S=" << startCount << ", G=" << goalCount << endl;
            return false;
        }

        return loadMapFromJson(filename, grid); 
    }
    catch (const exception& e) {
        cout << "Error al leer el JSON: " << e.what() << endl;
        return false;
    }
}
