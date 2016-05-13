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
bool Game::inFinalBattle = false;
bool Game::reqRegen = false;
AABB Game::building;
Transform* Game::lastBuilding;
Transform* Game::player;

Game::Game(Transform* player) {
    time.restart();

    // starting platform 
    // later add some old dude who talks to you XD
    AABB b(glm::vec3(-10.0f, 0.0f, -10.0f), glm::vec3(10.0f, 4.0f, 10.0f));
    Physics::addStatic(b);
    Transform* t = Graphics::registerTransform();
    t->setByBounds(b);
    t->color = glm::vec3(0.0f, 0.5f, 0.2f);

    lastBuilding = Graphics::registerTransform();
    lastBuilding->setVisibility(Visibility::HIDE_ALL);

    this->player = player;
}

void Game::reset() {
    timeCounter = 0.0f;
    flowRate = 0.0f;
    lavaTime = 0.0f;
    inFinalBattle = false;
    finalBattleMusicTimer = 0.0f;
    lastBuilding->setVisibility(Visibility::HIDE_ALL);
    auto& r = Resources::get();
    r.bossTrack.stop();
    if (r.mainTrack.getStatus() != sf::SoundSource::Status::Playing) {
        r.mainTrack.play();
    }
}

void Game::update(float delta) {
    timeCounter += delta;
    this->delta = delta;
    // calculate lava flow time
    int lavaInterval = 20;
    if (inFinalBattle || ((int)timeSinceStart() / lavaInterval) % 2 == 1 ||
        Input::pressed(sf::Keyboard::BackSpace)) {  // for debugging purposes
        flowRate += delta / flowSpeed;
    } else {
        flowRate -= delta / flowSpeed;
    }
    flowRate = Mth::saturate(flowRate);
    lavaTime += delta * flowRate;

    // delay play on music in final battle
    if (inFinalBattle) {
        finalBattleMusicTimer += delta;
        if (finalBattleMusicTimer > 5.0f) {
            Resources::get().bossTrack.play();
            finalBattleMusicTimer = -100000.0f;
        }
    }
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

void Game::startFinalBattle(AABB b) {
    if (inFinalBattle) {
        return;
    }

    Resources::get().mainTrack.stop();

    inFinalBattle = true;
    building = b;
}
bool Game::isInFinalBattle() {
    return inFinalBattle;
}

glm::vec3 Game::getFinalOrigin() {
    return lastBuilding->getWorldPos();
}

glm::vec3 Game::getPlayerPos() {
    return player->getWorldPos();
}

void Game::setRequiresWorldRegen(bool b) {
    reqRegen = b;
    if (!b) {   // called after world has been regenerated
        building.min.y = 0.0f;
        Physics::addStatic(building);
        lastBuilding->setByBounds(building);
        lastBuilding->setVisibility(Visibility::SHOW_SELF);
        lastBuilding->color = glm::vec3(0.0f, 0.5f, 0.25f);
    }
}
bool Game::requiresWorldRegen() {
    return reqRegen;
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