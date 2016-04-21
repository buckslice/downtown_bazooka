#include "game.h"
#include "input.h"
#include "mathutil.h"
#include "physics.h"
#include "graphics.h"

sf::Clock Game::time;
float Game::timeCounter = 0.0f;
float Game::delta = 0.0f;
float Game::flowRate = 0.0f;
float Game::lavaTime = 0.0f;   

Game::Game() {
    time.restart();

    // starting platform 
    // later add some old dude who talks to you XD
    AABB b(glm::vec3(-10.0f, 0.0f, -10.0f), glm::vec3(10.0f, 4.0f, 10.0f));
    Physics::addStatic(b);
    Transform* t = Graphics::registerTransform();
    t->setByBounds(b);
    t->color = glm::vec3(0.0f, 0.5f, 0.2f);

}

float Game::deltaTime() {
    return delta;
}

float Game::timeSinceStart() {
    return timeCounter;
}

float Game::realTimeSinceStart() {
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
    timeCounter += delta;
    this->delta = delta;
    // calculate lava flow time
    int lavaInterval = 20;
    if (((int)timeSinceStart() / lavaInterval) % 2 == 1 ||
        Input::pressed(sf::Keyboard::BackSpace)) {  // for debugging purposes
        flowRate += delta / flowSpeed;
    } else {
        flowRate -= delta / flowSpeed;
    }
    flowRate = Mth::saturate(flowRate);
    lavaTime += delta * flowRate;
}