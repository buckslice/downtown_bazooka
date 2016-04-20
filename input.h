#pragma once
#include <SFML/Window.hpp>
#include <SFML/Window/Keyboard.hpp>

const int KEYS = sf::Keyboard::KeyCount;
const int BUTTONS = sf::Mouse::ButtonCount;

class Input {
public:

    Input();
    void update();

    static bool pressed(sf::Keyboard::Key key);
    static bool justPressed(sf::Keyboard::Key key);
    static bool justReleased(sf::Keyboard::Key key);

    static bool pressed(sf::Mouse::Button button);
    static bool justPressed(sf::Mouse::Button button);
    static bool justReleased(sf::Mouse::Button button);

	static sf::Vector2i getMouseMovement(sf::Window& window, bool centerAndIgnore, sf::Vector2i center);

};