#include "UIHelper.h"
#include "GameConstants.h"
using namespace sf;


//pantalla de derrota
void showEndScreen(RenderWindow& window, const string& message, const Font& font) {
    // Forzar vista por defecto para evitar zoom
    window.setView(window.getDefaultView());

    // Crear texto grande y centrado
    Text endText;
    endText.setFont(font);
    endText.setCharacterSize(32); // Más grande para visibilidad
    endText.setFillColor(Color::White);
    endText.setString(message + "\n\nPresiona cualquier tecla para salir");

    // Centrar en pantalla real
    FloatRect textRect = endText.getLocalBounds();
    endText.setOrigin(textRect.width / 2.f, textRect.height / 2.f);
    Vector2u size = window.getSize();
    endText.setPosition(size.x / 2.f, size.y / 2.f);

    window.clear(Color::Black);
    window.draw(endText);
    window.display();

    // Esperar tecla
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
