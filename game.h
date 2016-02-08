#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <fstream>
#include <Windows.h>
#include <memory>

#include "shader.h"
#include "camera.h"
#include "cityGenerator.h"
#include "terrain.h"
#include "glHelper.h"
#include "physics.h"
#include "menu.h"
#include "game.h"
#include "graphics.h"
#include "input.h"
#include "enemy.h"
#include "mathutil.h"
#include "entityManager.h"

class Game {
public:
    Game(GLuint width, GLuint height);
    ~Game();

    void mainLoop();

    bool isRunning();
private:
    GLuint WIDTH;
    GLuint HEIGHT;
    sf::Vector2i center;
    sf::Music mainTrack;
    sf::RenderWindow* window;
    Graphics* graphics;
    Physics* physics;
    Input input;
    CityGenerator* cg;
    Terrain* tg;
    Camera cam;
    Player* player;
    EntityManager* em;
    Menu* menu;

    // some utility clocks
    sf::Clock frameTime;
    sf::Clock animTime;

    // state tracking flags
    bool running;
    bool lastFocused;
    bool gameFocused;
    bool clickedInside;
    bool mouseVisible;
    bool lastMouseVisible;
    bool mipmapping;
    bool blurring;
    bool wireframe;
};

