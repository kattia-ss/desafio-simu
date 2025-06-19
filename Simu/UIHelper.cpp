#include "UIHelper.h"
#include "GameConstants.h"
using namespace sf;


//pantalla de derrota
void showEndScreen(RenderWindow& window, const string& message, const Font& font) {
    Text endText;
    endText.setFont(font);
    endText.setCharacterSize(32);
    endText.setFillColor(Color::White);
    endText.setString(message + "\n\nPresiona CUALQUIER tecla para salir");

    // Centrar texto
    FloatRect textRect = endText.getLocalBounds();
    endText.setOrigin(textRect.width / 2.f, textRect.height / 2.f);
    endText.setPosition(WINDOW_WIDTH / 2.f, WINDOW_HEIGHT / 2.f);

    window.clear(Color::Black);
    window.draw(endText);
    window.display();

    // Esperar a que el usuario presione una tecla
    Event event;
    while (true) {
        while (window.pollEvent(event)) {
            if (event.type == Event::KeyPressed || event.type == Event::Closed) {
                window.close();
                return;
            }
        }
    }
}