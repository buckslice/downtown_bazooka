#include "menu.h"
#include <iostream>

sf::Text menu[MAX_NUMBER_OF_ITEMS];
sf::Text title;

Menu::Menu(float width, float height) {
    if (!font.loadFromFile("assets/fonts/cour.ttf")) {
        //handle error
        std::cout << "ERROR::FONT::LOAD_FAILURE";
    }
    selectedItemIndex = 0;
    lastUp = lastDown = false;
    menu[0].setFont(font);
    menu[0].setColor(selectedColor);
    menu[0].setString("Play");
    menu[0].setScale(sf::Vector2f(2.0f, 2.0f));
    menu[0].setPosition(sf::Vector2f(width / 2 - 65.0f, height / (MAX_NUMBER_OF_ITEMS + 1) * 1));

    menu[1].setFont(font);
    menu[1].setColor(defaultColor);
    menu[1].setString("Instructions");
    menu[1].setPosition(sf::Vector2f(width / 2 - 190.0f, height / (MAX_NUMBER_OF_ITEMS + 1) * 2));
    menu[1].setScale(sf::Vector2f(2.0f, 2.0f));

    menu[2].setFont(font);
    menu[2].setColor(defaultColor);
    menu[2].setString("Quit");
    menu[2].setPosition(sf::Vector2f(width / 2 - 65.0f, height / (MAX_NUMBER_OF_ITEMS + 1) * 3));
    menu[2].setScale(sf::Vector2f(2.0f, 2.0f));

    title = sf::Text();
    title.setFont(font);
    title.setColor(titleColor);
    title.setString("DOWTOWN BAZOOKA");
    title.setStyle(sf::Text::Bold);
    title.setPosition(sf::Vector2f(300.0f, height / (MAX_NUMBER_OF_ITEMS + 1) * 0.1));
    title.setScale(sf::Vector2f(3.0f, 3.0f));

    // this should give the width in pixels of the text so we can use that to center it
    // but too bad it crashes everytihng so hard lol whyyyyyy???
    //float w = title.getLocalBounds().width;
}


Menu::~Menu() {
}

void Menu::draw(sf::RenderWindow& window) {
    if (visible) {
        window.draw(title);
        for (int i = 0; i < MAX_NUMBER_OF_ITEMS; i++) {
            window.draw(menu[i]);
        }
    }
}

void Menu::moveUp() {
    menu[selectedItemIndex].setColor(defaultColor);
    if (selectedItemIndex > 0) {
        selectedItemIndex--;
    } else {
        selectedItemIndex = MAX_NUMBER_OF_ITEMS - 1;
    }
    menu[selectedItemIndex].setColor(selectedColor);
}

void Menu::setVisible(bool visible) {
    this->visible = visible;
}

bool Menu::getVisible() {
    return visible;
}

void Menu::showInstructions() {
    //TODO
}

void Menu::moveDown() {
    menu[selectedItemIndex].setColor(defaultColor);
    if (selectedItemIndex < MAX_NUMBER_OF_ITEMS - 1) {
        selectedItemIndex++;
    } else {
        selectedItemIndex = 0;
    }
    menu[selectedItemIndex].setColor(selectedColor);
}

void Menu::executeItem(sf::RenderWindow& window, bool& running, Game& game) {
    switch (selectedItemIndex) {
    case 0:
        setVisible(false);
        game.setRunning(true);
        break;
    case 1:
        showInstructions();
        break;
    case 2:
        running = false;
        window.close();
        break;
    }
}

void Menu::update(sf::RenderWindow& window, bool& running, Game& game) {
    if (visible && game.isRunning()) {
        game.setRunning(false);
    }

    draw(window);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
        if (!lastUp) {
            moveUp();
            lastUp = true;
        }
    } else {
        lastUp = false;
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
        if (!lastDown) {
            moveDown();
            lastDown = true;
        }
    } else {
        lastDown = false;
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Return) || sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
        executeItem(window, running, game);
    }
}