#include "game.h"
#include "input.h"
#include "mathutil.h"



sf::Clock Game::time;

float Game::delta = 0.0f;
float Game::flowRate = 0.0f;
float Game::lavaTime = 0.0f;   

Game::Game() {
    time.restart();
}

float Game::deltaTime() {
    return delta;
}

float Game::timeSinceStart() {
    return time.getElapsedTime().asSeconds();
}

bool Game::isGroundLava() {
    return flowRate > 0.5f;
}

float Game::getLavaFlowRate() {
    return flowRate;
}

float Game::getLavaTime() {
    return lavaTime;
}

void Game::update(float delta) {
    this->delta = delta;
    // calculate lava flow time
    int lavaInterval = 20;
    if (((int)timeSinceStart() / lavaInterval) % 2 == 1) {
        flowRate += delta / flowSpeed;
    } else {
        flowRate -= delta / flowSpeed;
    }
    flowRate = Mth::saturate(flowRate);
    lavaTime += delta * flowRate;
}