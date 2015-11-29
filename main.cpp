
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
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

// TODO extract to input class
sf::Vector2i getMouseMovement(sf::Window& window, bool lastFocus) {
    sf::Vector2i mouseMove;
    // get mouse movement
    if (window.hasFocus()) {
        // if just got focus this frame then ignore
        if (!lastFocus) {
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
    }
    return mouseMove;
}


int main() {
    // window settings
    sf::ContextSettings settings;
    settings.depthBits = 24;
    settings.stencilBits = 8;
    settings.antialiasingLevel = 2;

    //sf::Window window(sf::VideoMode(WIDTH, HEIGHT), "DOWNTOWN BAZOOKA", sf::Style::Close, settings);
    sf::RenderWindow* window = new sf::RenderWindow(sf::VideoMode(WIDTH, HEIGHT), "DOWNTOWN BAZOOKA", sf::Style::Close, settings);
    window->setFramerateLimit(60);

    Graphics graphics(*window);
    Input input;

    //Game game(WIDTH, HEIGHT);

    // init and build city
    CityGenerator cg;

    // init camera
    Camera cam(270.0f, 0.0f);
    Player* player = new Player(&cam);
    cam.transform.parent = player->transform;
    cam.transform.pos = glm::vec3(0.0f, 1.8f, 0.0f);

    Menu menu(WIDTH, HEIGHT, player);

    // set up some more stuff
    sf::Clock frameTime;
    sf::Clock animTime;
    sf::Mouse::setPosition(center, *window);
    window->setMouseCursorVisible(false);
    window->setKeyRepeatEnabled(false); // so sf::Event::KeyPressed will be useful
    bool lastFocused = false;
    bool windowFocused = false;

    // generate a random circular city
    cg.generate(false, 7500, graphics);

    Physics physics;
    physics.addStatics(cg.boxes);
    //physics.printStaticMatrix();

    EntityManager em(graphics, player);

    bool running = true;
    while (running) {
        GLfloat delta = frameTime.restart().asSeconds();

        lastFocused = windowFocused;
        windowFocused = window->hasFocus();

        // should check for window.resize event too and resize window
        // check for events that will quit game
        sf::Event e;
        while (window->pollEvent(e)) {
            switch (e.type) {
            case sf::Event::Closed:
                running = false;
                break;
            default:
                break;
            }
        }

        // update static input bool arrays
        if (windowFocused || lastFocused) {
            input.update();
        }

        // recompile shaders
        if (Input::justPressed(sf::Keyboard::R)) {
            graphics.buildShaders();
        }

        // build square or circular city if prompted
        if (Input::justPressed(sf::Keyboard::F) || Input::justPressed(sf::Keyboard::G)) {
            physics.clearStatics();
            cg.generate(Input::justPressed(sf::Keyboard::F), 7500, graphics);
            physics.addStatics(cg.boxes);
        }

        // get mouse movement and update based on window focus
        window->setMouseCursorVisible(!windowFocused);
        sf::Vector2i mouseMove = getMouseMovement(*window, lastFocused);

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

            if (windowFocused) {
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