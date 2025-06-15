#include <SFML/Graphics.hpp>
#include <iostream>

int main() {
	//crear ventana
	sf::RenderWindow window(sf::VideoMode(800, 600), "Escape the Grid - Test");
	//crear un hexagono
	sf::CircleShape hexagon(50, 6);
	hexagon.setFillColor(sf::Color::Green);
	hexagon.setPosition(375, 275); // centro de la ventana
	std::cout << "SFML configurado correctamente" << std::endl;
	std::cout << "Usa las flechas obiamente inutil" << std::endl;
	std::cout << "Presiona ESC para salir no seas pndj" << std::endl;

	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
			//input por teclado
			if (event.type == sf::Event::KeyPressed)
			{
				if (event.key.code == sf::Keyboard::Escape)
					window.close();
				//mover hexagono con las flechas
				if (event.key.code == sf::Keyboard::Up)
					hexagon.move(0, -10);
				if (event.key.code == sf::Keyboard::Down)
					hexagon.move(0, 10);
				if (event.key.code == sf::Keyboard::Left)
					hexagon.move(-10, 0);
				if (event.key.code == sf::Keyboard::Right)
					hexagon.move(10, 0);
			}
		}
		window.clear(sf::Color::Black);
		window.draw(hexagon);
		window.display();
	}


	return 0;
}