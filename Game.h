#pragma once

#include "Player.h"

class Game {
public:
    Game(int width, int height) {
        started = false;
        if (!font.loadFromFile("assets/fonts/MAGNETOB.ttf")) {
            //handle error
            std::cout << "ERROR::FONT::LOAD_FAILURE";
        }
        gameOver.setFont(font);
        gameOver.setColor(sf::Color::Red);
        gameOver.setString("Game Over");
        gameOver.setPosition(sf::Vector2f(width / 2 - 200.0f, height / 2));
        gameOver.setScale(sf::Vector2f(2.0f, 2.0f));
    }

    Player getPlayer() {
        return player;
    }

    bool gameIsOver() {
        return player.getHealth() == 0;
    }

    void setRunning(bool b) {
        started = b;
    }

    bool isRunning() {
        return started;
    }

    void update(sf::RenderWindow& window) {
        if (started) { //&& !gameIsOver()) {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
                player.addHealth(5);
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
                player.addHealth(-5);
            }
            player.update(window);
        }

        if (started && gameIsOver()) {
            //window.draw(gameOver);
        }
    }

private:
    Player player;
    bool started;
    sf::Font font;
    sf::Text gameOver;
};