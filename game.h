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
#include <queue>

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
    GLuint WIDTH;
    GLuint HEIGHT;
	float gameVolume;
    sf::Vector2i center;
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
	Audio* audio;

    // debug rendering stuff
    std::vector<glm::mat4>* dmodels;
    std::vector<glm::vec3>* dcolors;

    // some utility clocks
    sf::Clock frameTime;
    sf::Clock gameTime;

    sf::Text fpsText;
	sf::Text deadText;
    std::queue<float> fpsValues;
    float totalFpsQueueValue;

    void testMathUtils();
    void updateFpsText(float delta);

    sf::Vector2i mouseMove;
    GLfloat mouseScroll;

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
    bool paused;
    bool showFPS;
};

