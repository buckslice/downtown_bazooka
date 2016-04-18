#pragma once
#include "SFML\Graphics.hpp"

class Game {
public:
    Game::Game();

    void update(float delta);

    static float timeSinceStart();
    static float realTimeSinceStart();
    static float deltaTime();

    static bool isGroundLava();
    static float getLavaFlowRate();
    static float getLavaTime();

private:
    static sf::Clock time;      // real life time in seconds since game started
    static float timeCounter;   // adds and stores all deltas for total game tick time
    static float delta;         // save latest delta time each frame for convenience

    float flowSpeed = 3.0f; // how long in seconds to start flowing
    static float flowRate;  // value from 0-1 denoting transition period
    static float lavaTime;  // how much lava has flowed
};
