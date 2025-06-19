#pragma once
#include "HexagonCell.h"
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <utility>       
#include <functional>    
using namespace std;

struct Node {
    int row, col;
    float gCost, hCost;
    Node* parent;
    float fCost() const { return gCost + hCost; }
    bool operator>(const Node& other) const { return fCost() > other.fCost(); }
};

// Función hash para pair<int,int> para usar en unordered_set
struct PairHash {
    size_t operator()(const pair<int, int>& p) const {
        return hash<int>()(p.first) ^ (hash<int>()(p.second) << 1);
    }
};


float heuristic(int r1, int c1, int r2, int c2);

vector<pair<int, int>> getHexNeighbors(int row, int col, int maxRows, int maxCols);

vector<pair<int, int>> findPathAStar(
    vector<vector<HexagonCell>>& grid,
    int startR, int startC,
    int goalR, int goalC
);