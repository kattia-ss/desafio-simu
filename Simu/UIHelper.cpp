#include "UIHelper.h"
#include "GameConstants.h"
using namespace sf;


//pantalla de derrota
void showEndScreen(RenderWindow& window, const string& message, const Font& font) {
    window.setView(window.getDefaultView());
    Vector2u size = window.getSize();

    // Fondo gradiente dinámico según victoria o derrota
    Color topColor, bottomColor;
    if (message == "GAME OVER") {
        topColor = Color(40, 0, 20);       // rojo abisal oscuro
        bottomColor = Color(150, 20, 40);  // rojo profundo
    }
    else {
        topColor = Color(0, 50, 100);      // azul profundo
        bottomColor = Color(40, 200, 180); // turquesa brillante
    }

    VertexArray background(Quads, 4);
    background[0].position = Vector2f(0, 0);
    background[1].position = Vector2f(size.x, 0);
    background[2].position = Vector2f(size.x, size.y);
    background[3].position = Vector2f(0, size.y);

    background[0].color = topColor;
    background[1].color = topColor;
    background[2].color = bottomColor;
    background[3].color = bottomColor;

    // Texto principal
    Text endText;
    endText.setFont(font);
    endText.setCharacterSize(32);
    endText.setFillColor(message == "GAME OVER" ? Color(255, 80, 80) : Color(80, 255, 160));
    endText.setString(message);
    FloatRect textRect = endText.getLocalBounds();
    endText.setOrigin(textRect.width / 2.f, textRect.height / 2.f);
    endText.setPosition(size.x / 2.f, size.y / 2.f - 60);

    // Instrucción inferior
    Text hint;
    hint.setFont(font);
    hint.setCharacterSize(16);
    hint.setFillColor(Color::White);
    hint.setString("Presiona cualquier tecla para salir");
    FloatRect hintRect = hint.getLocalBounds();
    hint.setOrigin(hintRect.width / 2.f, hintRect.height / 2.f);
    hint.setPosition(size.x / 2.f, size.y / 2.f + 20);

    // Burbujas
    vector<CircleShape> bubbles;
    vector<float> speeds;
    for (int i = 0; i < 25; ++i) {
        CircleShape b(rand() % 5 + 2);
        b.setFillColor(Color(200, 255, 255, 120));
        b.setPosition(rand() % size.x, size.y + rand() % 300);
        bubbles.push_back(b);
        speeds.push_back(0.5f + rand() % 100 / 300.f);
    }

    Clock clock;
    while (window.isOpen()) {
        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::KeyPressed || event.type == Event::Closed) {
                window.close();
                return;
            }
        }

        for (size_t i = 0; i < bubbles.size(); ++i) {
            Vector2f pos = bubbles[i].getPosition();
            pos.y -= speeds[i];
            if (pos.y < -10) {
                pos.y = size.y + rand() % 200;
                pos.x = rand() % size.x;
            }
            bubbles[i].setPosition(pos);
        }

        window.clear();
        window.draw(background);
        for (auto& b : bubbles) window.draw(b);
        window.draw(endText);
        window.draw(hint);
        window.display();

        sleep(milliseconds(16));
    }
}

