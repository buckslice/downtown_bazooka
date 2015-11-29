#pragma once
#include <SFML/Window.hpp>
#include <SFML/Window/Keyboard.hpp>

const int KEYS = sf::Keyboard::KeyCount;

class Input {
public:

    Input();
    void update();

    static bool pressed(sf::Keyboard::Key key);
    static bool justPressed(sf::Keyboard::Key key);
    static bool justReleased(sf::Keyboard::Key key);

};