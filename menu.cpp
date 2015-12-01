#include "menu.h"
#include "input.h"
#include <iostream>

Menu::Menu(float width, float height, Player* player) {
    if (!font.loadFromFile("assets/fonts/cour.ttf")) {
        //handle error
        std::cout << "ERROR::FONT::LOAD_FAILURE";
    }
    curSelection = 0;

    menu[0].setFont(font);
    menu[0].setColor(SELECTED_COLOR);
    menu[0].setString("Play");
    menu[0].setScale(sf::Vector2f(2.0f, 2.0f));

    menu[1].setFont(font);
    menu[1].setColor(DEFAULT_COLOR);
    menu[1].setString("Instructions");
    menu[1].setScale(sf::Vector2f(2.0f, 2.0f));

    menu[2].setFont(font);
    menu[2].setColor(DEFAULT_COLOR);
    menu[2].setString("Quit");
    menu[2].setScale(sf::Vector2f(2.0f, 2.0f));

    title.setFont(font);
    title.setColor(TITLE_COLOR);
    title.setString("DOWNTOWN BAZOOKA");
    title.setStyle(sf::Text::Bold);
    title.setScale(sf::Vector2f(3.0f, 3.0f));

    healthBar.setFillColor(sf::Color(180, 255, 0, 255));

    this->player = player;

    // this should give the width in pixels of the text so we can use that to center it
    // but too bad it crashes the game so hard lol whyyyyyy???
    // some sort of weird access violation where we arent reseting states right??? bugged pos???
    //sf::FloatRect rect = title.getLocalBounds();

}

Menu::~Menu() {
}

void Menu::draw(sf::RenderWindow& window) {
    int width = window.getSize().x;
    int height = window.getSize().y;
    if (visible) {
        // set positions incase resize
        title.setPosition(sf::Vector2f(width / 2 - 440.0f, height / (MAX_NUMBER_OF_ITEMS + 1) * 0.3f));
        menu[0].setPosition(sf::Vector2f(width / 2 - 65.0f, height / (MAX_NUMBER_OF_ITEMS + 1) * 1.3f));
        menu[1].setPosition(sf::Vector2f(width / 2 - 190.0f, height / (MAX_NUMBER_OF_ITEMS + 1) * 2.3f));
        menu[2].setPosition(sf::Vector2f(width / 2 - 65.0f, height / (MAX_NUMBER_OF_ITEMS + 1) * 3.3f));

        window.draw(title);
        for (int i = 0; i < MAX_NUMBER_OF_ITEMS; i++) {
            window.draw(menu[i]);
        }
    } else {
        healthBar.setPosition(0.0f, height - HEALTH_BAR_HEIGHT);
        healthBar.setSize(sf::Vector2f(width * player->getHealth() / MAX_HEALTH, HEALTH_BAR_HEIGHT));
        window.draw(healthBar);
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
    menu[curSelection].setColor(DEFAULT_COLOR);
    curSelection += up ? -1 : 1;
    curSelection = circularClamp(curSelection, 0, MAX_NUMBER_OF_ITEMS - 1);
    menu[curSelection].setColor(SELECTED_COLOR);
}

void Menu::showInstructions() {
    //TODO
}

void Menu::update(bool& running) {
    justClosed = false;
    justOpened = false;
    if (Input::justPressed(sf::Keyboard::Escape)) {
        if (visible) {
            running = false;
        } else {
            visible = true;
            justOpened = true;
        }
    }

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
            justClosed = true;
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