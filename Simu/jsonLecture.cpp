#include "json.hpp"
#include <fstream>
#include <iostream>
#include <set>

using namespace std;
using json = nlohmann::json;

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
