
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
#include "glHelper.h"
#include "physics.h"
#include "menu.h"
#include "game.h"
#include "graphics.h"
#include "input.h"
#include "enemy.h"
#include "mathutil.h"
#include "entityManager.h"

sf::Vector2i center;

// TODO extract to input class
sf::Vector2i getMouseMovement(sf::Window& window, bool centerAndIgnore) {
    sf::Vector2i mouseMove;

    // if window just got refocused or on resize then recenter mouse and ignore
    if (centerAndIgnore) {
        sf::Mouse::setPosition(center, window);
        return sf::Vector2i(0, 0);
    }

    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    mouseMove = mousePos - center;

    // if the mouse has moved then set it back to center
    // needs to somehow prevent mouse going outside window in one frame
    // which is possible if you move fast enough
    if (mouseMove != sf::Vector2i(0, 0)) {
        sf::Mouse::setPosition(center, window);
    }
    return mouseMove;
}


int main() {

    // set default game width and height
    GLuint width = 1280;
    GLuint height = 960;
    center.x = width / 2;
    center.y = height / 2;

    // build window
    sf::ContextSettings settings;
    settings.depthBits = 24;
    settings.stencilBits = 8;
    settings.antialiasingLevel = 2;
    sf::RenderWindow* window = new sf::RenderWindow(sf::VideoMode(width, height), "DOWNTOWN BAZOOKA", sf::Style::Default, settings);
    window->setFramerateLimit(60);

    // load music track
    sf::Music mainTrack;
    if (!mainTrack.openFromFile("assets/music/expl1.ogg")) {
        std::cout << "ERROR::MUSIC_LOAD_FAILURE" << std::endl;
    }

    // main systems
    Graphics graphics(*window);
    Physics physics;
    Input input;

    // init random seed and build city
    srand(time(NULL));
    //srand(1); //my testing seed for physics bugs lol
    CityGenerator cg;

    // generate a random city
    cg.generate(false, false, 7500, graphics, physics);

    // init camera
    Camera cam(0.0f, 0.0f);
    Player* player = new Player(&cam);
    cam.transform.parent = player->transform;
    cam.transform.pos = glm::vec3(0.0f, 1.8f, 0.0f);

    EntityManager em(graphics, player);
    
    Menu menu(width, height, player);

    // set up music
    mainTrack.setLoop(true);
    mainTrack.setVolume(20.0f);
    mainTrack.play();

    // some utility clocks
    sf::Clock frameTime;
    sf::Clock animTime;

    // mouse and window focusing variables
    sf::Mouse::setPosition(center, *window);
    window->setMouseCursorVisible(false);
    window->setKeyRepeatEnabled(false); // so sf::Event::KeyPressed will be useful
    bool lastFocused = false;
    bool gameFocused = false;
    bool clickedInside = true;
    bool mouseVisible = false;
    bool lastMouseVisible = false;

    bool running = true;

    while (running) {
        GLfloat delta = frameTime.restart().asSeconds();

        // should check for window.resize event too and resize window?
        // check for events that will quit game
        sf::Event e;
        while (window->pollEvent(e)) {
            switch (e.type) {
            case sf::Event::Closed:
                running = false;
                break;
            case sf::Event::Resized:
                window->setView(sf::View(sf::FloatRect(0.0f, 0.0f, e.size.width, e.size.height)));
                graphics.resize(e.size.width, e.size.height);
                center.x = e.size.width / 2;
                center.y = e.size.height / 2;
                break;
            default:
                break;
            }
        }

        // check and track game focusing
        lastFocused = gameFocused;
        gameFocused = window->hasFocus() && clickedInside;
        if (!gameFocused && lastFocused) {
            clickedInside = false;
        }
        if (!clickedInside && sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
            sf::Vector2i mp = sf::Mouse::getPosition(*window);
            clickedInside = mp.x >= 0 && mp.y >= 0;
        }

        // handle mouse visibility
        lastMouseVisible = mouseVisible;
        mouseVisible = !gameFocused;
        if (mouseVisible != lastMouseVisible) {
            // if you call this every frame it messes up mouse icon
            window->setMouseCursorVisible(mouseVisible);
        }

        // get mouse movement and update based on window focus
        sf::Vector2i mouseMove;
        if (gameFocused) {
            mouseMove = getMouseMovement(*window, !lastFocused);
        }

        // update static input bool arrays
        if (gameFocused || lastFocused) {
            input.update();
        }

        // recompile shaders if prompted
        if (Input::justPressed(sf::Keyboard::R)) {
            graphics.buildShaders();
        }

        // build square or circular city if prompted
        if (Input::justPressed(sf::Keyboard::F) || Input::justPressed(sf::Keyboard::G)) {
            physics.clearStatics();
            cg.generate(false, Input::justPressed(sf::Keyboard::F), 7500, graphics, physics);
        }

        //update menu
        menu.update(running);

        if (menu.justClosed) {
            player->flying = false;
            em.init(2000);
        }
        if (menu.justOpened) {
            em.deleteEntities(1);
            physics.clearDynamics();
        }

        // update camera
        if (!menu.getVisible()) {   // if game running
            em.update(delta);

            if (gameFocused) {
                cam.update(mouseMove.x, mouseMove.y);
            }
        } else {
            // reset player to floating above city
            player->setPosition(glm::vec3(0.0f, 200.0f, 0.0f));
            player->getTransform()->vel = glm::vec3(0.0f);
            // auto scroll camera
            cam.pitch = 0.0f;
            cam.yaw += 5.0f * delta;
            cam.updateCameraVectors();
        }

        // resolve collisions
        physics.update(delta);

        // render graphics
        graphics.renderScene(cam, !menu.getVisible());

        // draw UI
        window->resetGLStates();
        menu.draw(*window);

        // apply bloom
        graphics.postProcess();

        // swap buffers
        window->display();
    }

    return 0;
}