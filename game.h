#pragma once
#include "SFML\Graphics.hpp"
#include "transform.h"

class Game {
public:
    Game::Game();

    void update(float delta);

    void reset();

    static float deltaTime();
    static float timeSinceStart();
    static float realTimeSinceStart();

    static void startFinalBattle(AABB b);
    static bool isInFinalBattle();

    static void setRequiresWorldRegen(bool b);
    static bool requiresWorldRegen();

    static bool isGroundLava();
    static float getLavaFlowRate();
    static float getLavaTime();

private:
    float finalBattleMusicTimer = 0.0f;

    static Transform* lastBuilding;
    static AABB building;

    static sf::Clock time;      // real life time in seconds since game started
    static float timeCounter;   // adds and stores all deltas for total game tick time
    static float delta;         // save latest delta time each frame for convenience

    const float flowSpeed = 3.0f; // how long in seconds to start flowing
    static float flowRate;  // value from 0-1 denoting transition period
    static float lavaTime;  // how much lava has flowed
    static bool inFinalBattle;  // whether or in final battle stage

    static bool reqRegen;   // whether or not world should be regenerated
};
