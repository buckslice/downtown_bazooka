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
    Terrain* terrain;
    Camera cam;
    Player* player;
    EntityManager* entityManager;
    Menu* menu;
    Audio* audio;

    // some utility clocks
    sf::Clock frameTime, gameTime;

    void testMathUtils();

    // for mouse input
    sf::Vector2i mouseMove;
    GLfloat mouseScroll;

    // state tracking flags
    bool running = true;
    bool lastFocused = false;
    bool gameFocused = false;
    bool clickedInside = true;
    bool mouseVisible = false;
    bool lastMouseVisible = false;
    bool mipmapping = true;
    bool blurring = true;
    bool wireframe = false;
    bool paused = false;
    bool showFPS = false;;
};

