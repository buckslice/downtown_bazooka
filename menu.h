#pragma once

#include "SFML\Graphics.hpp"
#include "game.h"

#define MAX_NUMBER_OF_ITEMS 3
#define defaultColor sf::Color::White
#define titleColor sf::Color::Yellow
#define selectedColor sf::Color::Green

class Menu {
public:
    Menu(float width, float height);
    ~Menu();

    void draw(sf::RenderWindow& window);
    void moveUp();
    void moveDown();
    void update(sf::RenderWindow& window, bool& running, Game& game);
    void setVisible(bool visible);
    bool getVisible();
    void showInstructions();
private:
    int selectedItemIndex;
    sf::Font font;

    bool lastUp;
    bool lastDown;
    bool visible;
    void executeItem(sf::RenderWindow& window, bool& running, Game& game);
};

