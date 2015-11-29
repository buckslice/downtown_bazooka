#pragma once

#include "player.h"
#include "input.h"

class Game {
public:
    Game(int width, int height) {
        running = false;

    }

    bool gameIsOver() {
    }

    void setRunning(bool b) {
        running = b;
    }

    bool isRunning() {
        return running;
    }

    void update(sf::RenderWindow& window) {

    }

private:
    bool running;
};