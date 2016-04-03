#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <Windows.h>
#include <memory>

#include "audio.h"
#include "shader.h"
#include "camera.h"
#include "cityGenerator.h"
#include "terrain.h"
#include "glHelper.h"
#include "physics.h"
#include "menu.h"
#include "graphics.h"
#include "input.h"
#include "enemy.h"
#include "mathutil.h"
#include "entityManager.h"

class Game {
public:
    Game(GLuint width, GLuint height);
    ~Game();

    void start();

    void pollEvents();

    void getInput();

    void toggleOptions();

    void update(GLfloat delta);

    void render();


private:
    const GLuint NUM_BUILDINGS = 1000; // 7500
    const GLuint NUM_ENEMIES = 1000;

    GLuint WIDTH, HEIGHT;
    sf::Vector2i center;
    sf::RenderWindow* window;
    Graphics* graphics;
    Physics* physics;
    Input input;
    CityGenerator* cityGen;
    Terrain* terrainGen;
    Camera cam;
    Player* player;
    EntityManager* entityManager;
    Menu* menu;
	Audio* audio;

    // debug rendering stuff
    std::vector<glm::mat4>* dmodels;
    std::vector<glm::vec3>* dcolors;

    // some utility clocks
    sf::Clock frameTime, gameTime;

    void testMathUtils();

	// for mouse input
    sf::Vector2i mouseMove;
    GLfloat mouseScroll;

    // state tracking flags
	bool running, lastFocused, gameFocused, clickedInside, mouseVisible, lastMouseVisible, mipmapping, blurring, wireframe, paused, showFPS;
};

