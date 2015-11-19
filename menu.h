#pragma once

#include "SFML\Graphics.hpp"

#define MAX_NUMBER_OF_ITEMS 3
#define defaultColor sf::Color::White
#define titleColor sf::Color::Yellow
#define selectedColor sf::Color::Green

class Menu {
public:
    Menu(float width, float height);
    ~Menu();

    void draw(sf::RenderWindow& window);
    void update(bool& running);
    void setVisible(bool visible);
    bool getVisible();

private:
    void move(bool up);
    void showInstructions();

    sf::Text menu[MAX_NUMBER_OF_ITEMS];
    sf::Text title;

    sf::Font font;
    int curSelection;
    bool visible = true;
};

