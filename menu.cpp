#include "menu.h"
#include "input.h"
#include <iostream>

Menu::Menu(float width, float height) {
    if (!font.loadFromFile("assets/fonts/cour.ttf")) {
        //handle error
        std::cout << "ERROR::FONT::LOAD_FAILURE";
    }
    curSelection = 0;

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

int circularClamp(int n, int min, int max) {
    if (n < min) {
        return max;
    } else if (n > max) {
        return min;
    }
    return n;
}

void Menu::move(bool up) {
    menu[curSelection].setColor(defaultColor);
    curSelection += up ? -1 : 1;
    curSelection = circularClamp(curSelection, 0, MAX_NUMBER_OF_ITEMS - 1);
    menu[curSelection].setColor(selectedColor);
}

void Menu::showInstructions() {
    //TODO
}

void Menu::update(bool& running) {
    if (!visible) {
        return;
    }

    if (Input::justPressed(sf::Keyboard::W) || Input::justPressed(sf::Keyboard::Up)) {
        move(true);
    }

    if (Input::justPressed(sf::Keyboard::S) || Input::justPressed(sf::Keyboard::Down)) {
        move(false);
    }

    if (Input::justPressed(sf::Keyboard::Return) || Input::justPressed(sf::Keyboard::Space)) {
        switch (curSelection) {
        case 0:
            setVisible(false);
            break;
        case 1:
            showInstructions();
            break;
        case 2:
            running = false;
            break;
        }
    }
}

void Menu::setVisible(bool visible) {
    this->visible = visible;
}

bool Menu::getVisible() {
    return visible;
}