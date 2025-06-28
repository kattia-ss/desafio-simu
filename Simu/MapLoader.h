#pragma once

#include <string>
#include <vector>
#include "HexagonCell.h" 

using namespace std;



bool loadMapFromJson(const string& filename, vector<vector<HexagonCell>>& grid);