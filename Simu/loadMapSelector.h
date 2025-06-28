	#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <fstream>
#include "json.hpp"
#include "GameConstants.h"
#include "HexagonCell.h"

using namespace std;

bool loadMapWithValidation(const string& filename, vector<vector<HexagonCell>>& grid);