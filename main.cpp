
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


const GLuint WIDTH = 1440, HEIGHT = 960;
const sf::Vector2i center(WIDTH / 2, HEIGHT / 2);

FBO blurBuffers[2];
const GLuint BLUR_DOWNSAMPLE = 2;

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

// render a fullscreen quad
// used to render into frame buffers
GLuint quadVAO = 0;
void renderQuad() {
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

// blur a color buffer for a given number of iterations
void blurColorBuffer(GLuint sceneIn, GLuint frameOut, GLuint iterations, Shader screen, Shader blur) {

    // downsample into first blur buffer
    screen.use();
    glViewport(0, 0, WIDTH / BLUR_DOWNSAMPLE, HEIGHT / BLUR_DOWNSAMPLE);
    glBindTexture(GL_TEXTURE_2D, sceneIn);
    glBindFramebuffer(GL_FRAMEBUFFER, blurBuffers[0].frame);
    renderQuad();

    blur.use();
    GLboolean horizontal = true;
    GLuint radLoc = glGetUniformLocation(blur.program, "radius");
    GLuint resLoc = glGetUniformLocation(blur.program, "resolution");
    GLuint dirLoc = glGetUniformLocation(blur.program, "dir");

    // nice weird numbers that dont line up lol
    GLuint lookups[4] = { 1, 3, 7, 13 };
    //GLuint lookups[4] = { 2, 5, 9, 17 };

    // iterations * 2 since does width then height
    // could have simplified below code but left it
    // incase we want directional blurs later
    for (GLuint i = 0; i < iterations * 2; i++) {
        GLuint blurRadius = i / 2 + 1;
        //blurRadius = pow(2, (i / 2 + 1)); //or pow(3,
        if (iterations <= 4) {
            blurRadius = lookups[i / 2];
        }

        glUniform1f(radLoc, blurRadius);
        glUniform1f(resLoc, horizontal ? WIDTH : HEIGHT);
        GLfloat hz = horizontal ? 1.0f : 0.0f;
        glUniform2f(dirLoc, hz, 1.0f - hz);
        glBindTexture(GL_TEXTURE_2D, blurBuffers[!horizontal].color);
        glBindFramebuffer(GL_FRAMEBUFFER, blurBuffers[horizontal].frame);

        renderQuad();

        horizontal = !horizontal;
    }

    // upsample into blur result buffer
    screen.use();
    glViewport(0, 0, WIDTH, HEIGHT);
    glBindTexture(GL_TEXTURE_2D, blurBuffers[!horizontal].color);
    glBindFramebuffer(GL_FRAMEBUFFER, frameOut);
    renderQuad();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);	//back to default framebuffer
}

sf::RenderWindow* initGL() {
    // window settings
    sf::ContextSettings settings;
    settings.depthBits = 24;
    settings.stencilBits = 8;
    settings.antialiasingLevel = 2;

    //sf::Window window(sf::VideoMode(WIDTH, HEIGHT), "DOWNTOWN BAZOOKA", sf::Style::Close, settings);
    sf::RenderWindow* window = new sf::RenderWindow(sf::VideoMode(WIDTH, HEIGHT), "DOWNTOWN BAZOOKA", sf::Style::Close, settings);
    window->setFramerateLimit(60);

    // init glew (must be after window creation)
    glewExperimental = GL_TRUE;
    glewInit();

    // setup OpenGL options
    glViewport(0, 0, WIDTH, HEIGHT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // initialize blur color buffers
    for (GLuint i = 0; i < 2; i++) {
        blurBuffers[i] = GLHelper::buildFBO(WIDTH / BLUR_DOWNSAMPLE, HEIGHT / BLUR_DOWNSAMPLE, false);
    }

    // initialize quad for framebuffer rendering
    GLfloat quadVertices[] = {
        // Positions        // Texture Coords
        -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
        1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
        1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
    };
    glGenVertexArrays(1, &quadVAO);
    GLuint quadVBO;
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));

    return window;
}

int main() {
    sf::RenderWindow* window = initGL();

    Menu menu(WIDTH, HEIGHT);
    Game game(WIDTH, HEIGHT);

    // build and compile shaders
    // basic shader for buildings
    Shader buildingShader("assets/shaders/instanced.vert", "assets/shaders/default.frag");
    buildingShader.use();
    glUniform1i(glGetUniformLocation(buildingShader.program, "tex"), 0);

    // shader that blurs a colorbuffer
    Shader blurShader("assets/shaders/screen.vert", "assets/shaders/blur.frag");
    Shader screenShader("assets/shaders/screen.vert", "assets/shaders/screen.frag");
    screenShader.use();
    glUniform1i(glGetUniformLocation(screenShader.program, "screen"), 0);

    // shader that blends the blur with the scene
    Shader blendShader("assets/shaders/screen.vert", "assets/shaders/blend.frag");
    blendShader.use();
    glUniform1i(glGetUniformLocation(blendShader.program, "scene"), 0);
    glUniform1i(glGetUniformLocation(blendShader.program, "blur"), 1);

    GLuint tex = GLHelper::loadTexture("assets/images/grid.png");

    CityGenerator cg;
    Mesh mesh = cg.buildMesh(tex);

    // build main frame buffer
    // holds color and depth data
    FBO sceneBuffer = GLHelper::buildFBO(WIDTH, HEIGHT, true);
    FBO blurResult = GLHelper::buildFBO(WIDTH, HEIGHT, true);

    // save matrix locations from shader
    GLint projLoc = glGetUniformLocation(buildingShader.program, "proj");
    GLint viewLoc = glGetUniformLocation(buildingShader.program, "view");

    // init camera and projection
    Camera cam(0.0f, 200.0f, 0.0f, 0.0f, 1.0f, 0.0f, 270.0f, 0.0f);
    glm::mat4 proj = glm::perspective(45.0f, (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 2000.0f);

    // set up some more stuff
    sf::Clock frameTime;
    sf::Clock animTime;
    sf::Mouse::setPosition(center, *window);
    window->setMouseCursorVisible(false);
    window->setKeyRepeatEnabled(false); // so sf::Event::KeyPressed will be useful
    glClearColor(.05f, .0f, 0.1f, 1.0f); // set black clear color
    Physics physics(cam);
    bool lastFocus = false;

    // generate a random city
    cg.generateModelMatrices(false);
    cg.uploadModelMatrices(mesh);
    physics.addObjects(cg.boxes);

    bool running = true;
    while (running) {
        GLfloat deltaTime = frameTime.restart().asSeconds();

        // check for events that will quit game
        sf::Event e;
        while (window->pollEvent(e)) {
            switch (e.type) {
            case sf::Event::Closed:
                running = false;
                break;
            case sf::Event::KeyPressed: // triggers first time a key is pressed
                bool success = true;
                switch (e.key.code) {
                case sf::Keyboard::Escape:
                    if (menu.getVisible()) {
                        running = false;
                    } else {
                        menu.setVisible(true);
                    }
                    break;
                case sf::Keyboard::R:
                    glDeleteProgram(buildingShader.program);
                    glDeleteProgram(blurShader.program);
                    glDeleteProgram(screenShader.program);
                    glDeleteProgram(blendShader.program);

                    success = success && buildingShader.build("assets/shaders/instanced.vert", "assets/shaders/default.frag");
                    buildingShader.use();
                    glUniform1i(glGetUniformLocation(buildingShader.program, "tex"), 0);

                    // shader that blurs a colorbuffer
                    success = success && blurShader.build("assets/shaders/screen.vert", "assets/shaders/blur.frag");
                    success = success && screenShader.build("assets/shaders/screen.vert", "assets/shaders/screen.frag");
                    screenShader.use();
                    glUniform1i(glGetUniformLocation(screenShader.program, "screen"), 0);

                    // shader that blends the blur with the scene
                    success = success && blendShader.build("assets/shaders/screen.vert", "assets/shaders/blend.frag");
                    blendShader.use();
                    glUniform1i(glGetUniformLocation(blendShader.program, "scene"), 0);
                    glUniform1i(glGetUniformLocation(blendShader.program, "blur"), 1);

                    std::cout << "SHADERS::RECOMPILE::" << (success ? "SUCCESS" : "FAILURE") << std::endl;
                    break;
                case sf::Keyboard::F:        // generate new square city
                    physics.clearObjects();
                    cg.generateModelMatrices(true);
                    physics.addObjects(cg.boxes);
                    cg.uploadModelMatrices(mesh);
                    break;
                case sf::Keyboard::G:        // generate new circular city
                    physics.clearObjects();
                    cg.generateModelMatrices(false);
                    physics.addObjects(cg.boxes);
                    cg.uploadModelMatrices(mesh);
                    break;
                case sf::Keyboard::Q:
                    cam.flying = !cam.flying;
                    break;
                case sf::Keyboard::Space:
                    cam.jump();
                    break;
                default:
                    break;
                }

                break;
            }
        }

        // get mouse movement and update based on window focus
        window->setMouseCursorVisible(!window->hasFocus());
        sf::Vector2i mouseMove = getMouseMovement(*window, lastFocus);
        lastFocus = window->hasFocus();

        // update camera
        if (game.isRunning()) {
            cam.update(getMovementDir(), mouseMove.x, mouseMove.y, deltaTime);
        } else {
            //TODO make camera AI to scroll around the city
            cam.pos = glm::vec3(0.0f, 200.0f, 0.0f);
            cam.pitch = 0.0f;
            cam.yaw += 5.0f * deltaTime;
            cam.updateCameraVectors();
        }

        // update physics
        physics.update(deltaTime);

        // RENDER SCENE TO FRAMEBUFFER
        glBindFramebuffer(GL_FRAMEBUFFER, sceneBuffer.frame);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        buildingShader.use();
        // set projection and view matrices
        glm::mat4 view = cam.getViewMatrix();
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // draw instanced mesh a bunch of times (sets model matrix internally)
        mesh.draw(buildingShader, NUMBER_OF_MESHES);

        // prepare openGL states for UI rendering
        glBindVertexArray(0);
        glUseProgram(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        window->resetGLStates();

        // update and render UI
        menu.update(*window, running, game);
        game.update(*window);

        // BLUR PASS
        glDisable(GL_DEPTH_TEST);	//dont need this now
        blurColorBuffer(sceneBuffer.color, blurResult.frame, 4, screenShader, blurShader);

        // FINAL PASS (combines blur buffer with original scene buffer)
        glClear(GL_COLOR_BUFFER_BIT);
        blendShader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, sceneBuffer.color);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, blurResult.color);
        // blur strength is how bright the blur is
        glUniform1f(glGetUniformLocation(blendShader.program, "blurStrength"), 3.0f);
        renderQuad();

        // swap buffers
        window->display();
    }

    // clean up
    glDeleteTextures(1, &tex);
    glDeleteProgram(buildingShader.program);
    glDeleteProgram(blurShader.program);
    glDeleteProgram(screenShader.program);
    glDeleteProgram(blendShader.program);
    cg.destroy();
    //delete window;

    return 0;
}