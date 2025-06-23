#ifndef FILESELECTOR_H
#define FILESELECTOR_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <fstream>
#include "json.hpp"
#include "GameConstants.h"

using namespace std;
using namespace sf;
using json = nlohmann::json;

class FileSelector {
private:
    vector<string> jsonFiles;
    size_t selectedIndex;
    bool showSelector;
    string selectedFile;

public:
    FileSelector();
    void scanJsonFiles();
    void handleInput(const Event& event);
    void draw(RenderWindow& window, const Font& font);
    string getSelectedFile() const;
    void show();
    void hide();
    bool isVisible() const;
    bool validateJsonStructure(const string& filename);
};

#endif