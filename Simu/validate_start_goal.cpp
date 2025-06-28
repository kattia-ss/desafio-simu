#include "HexagonCell.h"
#include <vector>
#include <iostream>

bool validateStartAndGoal(const std::vector<std::vector<HexagonCell>>& grid) {
    int startCount = 0;
    int goalCount = 0;

    for (const auto& row : grid) {
        for (const auto& cell : row) {
            if (cell.isStart) startCount++;
            if (cell.isGoal) goalCount++;
        }
    }

    if (startCount != 1 || goalCount != 1) {
        std::cerr << "❌ Error de validación del mapa:" << std::endl;
        std::cerr << "   Se encontraron " << startCount << " celdas de inicio (S)" << std::endl;
        std::cerr << "   Se encontraron " << goalCount << " celdas de meta (G)" << std::endl;
        std::cerr << "   El mapa debe tener exactamente UNA celda 'S' y UNA celda 'G'" << std::endl;
        return false;
    }

    return true;
}