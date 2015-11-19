
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <Windows.h>

#include "shader.h"
#include "camera.h"
#include "cityGenerator.h"
#include "glHelper.h"
#include "physics.h"
#include "menu.h"
#include "game.h"
#include "graphics.h"
#include "input.h"

glm::vec3 getMovementDir() {
    // calculate movement direction
    glm::vec3 dir(0.0f, 0.0f, 0.0f);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Comma) || sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
        dir.z += 1.0f;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::O) || sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
        dir.z -= 1.0f;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
        dir.x -= 1.0f;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::E) || sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
        dir.x += 1.0f;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) {
        dir.y -= 1.0f;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
        dir.y += 1.0f;
    }
    return dir;
}

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

    Menu menu(WIDTH, HEIGHT);
    Game game(WIDTH, HEIGHT);

    // init and build city
    CityGenerator cg;
    GLuint tex = GLHelper::loadTexture("assets/images/grid.png");
    Mesh mesh = cg.buildMesh(tex);
    graphics.buildingMesh = &mesh;

    // init camera
    Camera cam(0.0f, 200.0f, 0.0f, 0.0f, 1.0f, 0.0f, 270.0f, 0.0f);
    float timeSinceJump = -1.0f;

    // set up some more stuff
    sf::Clock frameTime;
    sf::Clock animTime;
    sf::Mouse::setPosition(center, *window);
    window->setMouseCursorVisible(false);
    window->setKeyRepeatEnabled(false); // so sf::Event::KeyPressed will be useful
    Physics physics(cam);
    bool lastFocus = false;

    // generate a random city
    cg.generateModelMatrices(false);
    cg.uploadModelMatrices(mesh);
    physics.addObjects(cg.boxes);

    bool running = true;
    while (running) {
        GLfloat deltaTime = frameTime.restart().asSeconds();

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
        input.update();

        if (Input::justPressed(sf::Keyboard::Escape)) {
            if (menu.getVisible()) {
                running = false;
            } else {
                menu.setVisible(true);
            }
        }

        // recompile shaders
        if (Input::justPressed(sf::Keyboard::R)) {
            graphics.buildShaders();
        }

        // build square city
        if (Input::justPressed(sf::Keyboard::F)) {
            physics.clearObjects();
            cg.generateModelMatrices(true);
            physics.addObjects(cg.boxes);
            cg.uploadModelMatrices(mesh);
        }

        // build circular city
        if (Input::justPressed(sf::Keyboard::G)) {
            physics.clearObjects();
            cg.generateModelMatrices(false);
            physics.addObjects(cg.boxes);
            cg.uploadModelMatrices(mesh);
        }

        // toggle flying
        if (Input::justPressed(sf::Keyboard::Q)) {
            cam.flying = !cam.flying;
        }

        // jumping
        if (Input::justPressed(sf::Keyboard::Space)) {
            timeSinceJump = 0.0f;
        }

        float jumpLenience = 0.2f;
        if (timeSinceJump < jumpLenience) {
            cam.jump();
        }
        timeSinceJump += deltaTime;

        // get mouse movement and update based on window focus
        window->setMouseCursorVisible(!window->hasFocus());
        sf::Vector2i mouseMove = getMouseMovement(*window, lastFocus);
        lastFocus = window->hasFocus();

        //update menu
        menu.update(running);
        game.setRunning(!menu.getVisible());

        // update camera
        if (game.isRunning()) {
            cam.update(getMovementDir(), mouseMove.x, mouseMove.y, deltaTime);
        } else {
            //TODO make camera AI to scroll around the city
            cam.pos = glm::vec3(0.0f, 200.0f, 0.0f);
            cam.vel = glm::vec3(0.0f);
            cam.pitch = 0.0f;
            cam.yaw += 5.0f * deltaTime;
            cam.updateCameraVectors();
        }

        // update entitys here

        // resolve collisions
        physics.update(deltaTime);

        // render graphics
        graphics.renderScene(cam);

        // draw UI
        window->resetGLStates();
        menu.draw(*window);
        game.update(*window);

        // apply bloom
        graphics.postProcess();

        // swap buffers
        window->display();
    }

    // clean up
    glDeleteTextures(1, &tex);
    cg.destroy();
    //delete window;

    return 0;
}