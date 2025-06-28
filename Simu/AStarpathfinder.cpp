#include "AStarPathfinder.h"
#include <cmath>
#include <algorithm>

using namespace std;


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