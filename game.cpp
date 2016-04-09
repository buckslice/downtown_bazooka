#include "game.h"

Game::Game(GLuint width, GLuint height)
    : WIDTH{ width }, HEIGHT{ height } {

    center.x = WIDTH / 2;
    center.y = HEIGHT / 2;
    // build window
    sf::ContextSettings settings;
    settings.depthBits = 24;
    settings.stencilBits = 8;
    settings.antialiasingLevel = 2;
    window = new sf::RenderWindow(sf::VideoMode(WIDTH, HEIGHT), "DOWNTOWN BAZOOKA", sf::Style::Default, settings);
    window->setFramerateLimit(60);

    // main systems
    graphics = new Graphics(*window);
    Resources::get();   // makes sure to load resources
    physics = new Physics();
    terrain = new Terrain();
    physics->terrainGen = terrain;

    // init camera
    player = new Player(&cam);
    auto pt = player->getTransform();
    cam.transform->setPos(glm::vec3(0.0f, 1.8f, 0.0f));
    pt->parentAll(cam.transform);

    entityManager = new EntityManager(player);

    menu = new Menu(player);
    window->resetGLStates();

    audio = new Audio();
    Resources::get().menuTrack.play();

    // mouse and window focusing variables
    sf::Mouse::setPosition(center, *window);
    window->setMouseCursorVisible(false);
    window->setKeyRepeatEnabled(false); // so sf::Event::KeyPressed will be more accurate

    const GLuint MAX_DEBUG = 20000;
    dmodels = new std::vector<glm::mat4>(MAX_DEBUG);
    dcolors = new std::vector<glm::vec3>(MAX_DEBUG);

}

void Game::start() {
    gameTime.restart();  // holds time since start of game
    frameTime.restart();
    while (running) {

        GLfloat delta = frameTime.restart().asSeconds();

        pollEvents();

        getInput();

        toggleOptions();

        update(delta);

        render();

    }
}

void Game::pollEvents() {
    mouseScroll = 0.0f;

    sf::Event e;
    while (window->pollEvent(e)) {
        switch (e.type) {
        case sf::Event::Closed:
            running = false;
            break;
        case sf::Event::Resized:
        {   // need these brackets cuz c++ switches are wacky
            float w = static_cast<float>(e.size.width);
            float h = static_cast<float>(e.size.height);
            window->setView(sf::View(sf::FloatRect(0.0f, 0.0f, w, h)));
            graphics->resize(e.size.width, e.size.height);
            center.x = e.size.width / 2;
            center.y = e.size.height / 2;
            break;
        }
        case sf::Event::MouseWheelScrolled:
            if (e.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel) {
                mouseScroll -= e.mouseWheelScroll.delta;
            }
            break;
        default:
            break;
        }
    }
}

void Game::getInput() {
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
    mouseMove = sf::Vector2i(0, 0);
    if (gameFocused) {
        mouseMove = input.getMouseMovement(*window, !lastFocused, center);
    }

    // update static input bool arrays
    if (gameFocused || lastFocused) {
        input.update();
    }
}


void Game::toggleOptions() {
    // toggle blur
    if (Input::justPressed(sf::Keyboard::Num1)) {
        blurring = !blurring;
        std::cout << "BLUR " << (blurring ? "ON" : "OFF") << std::endl;
    }
    // toggle mipmaps
    if (Input::justPressed(sf::Keyboard::Num2)) {
        mipmapping = !mipmapping;
        Resources::get().loadTextures(mipmapping);
        std::cout << "MIPMAPS " << (mipmapping ? "ON" : "OFF") << std::endl;
    }
    // toggle collider wireframe view
    if (Input::justPressed(sf::Keyboard::Num3)) {
        wireframe = !wireframe;
        blurring = !wireframe;
        mipmapping = !wireframe;
        Resources::get().loadTextures(mipmapping);
        //physics->setColliderTransformVisibility(!wireframe);
        std::cout << "PHYSICS DEBUG " << (wireframe ? "ON" : "OFF") << std::endl;
    }
    // toggle terrain color debug
    if (Input::justPressed(sf::Keyboard::Num4)) {
        terrain->deleteChunks();
        std::cout << "TERRAIN DEBUG " << (terrain->toggleDebugColors() ? "ON" : "OFF") << std::endl;
    }
    // randomize world seed
    if (Input::justPressed(sf::Keyboard::Num5)) {
        terrain->deleteChunks();
        terrain->setSeed(glm::vec2(Mth::randRange(-10000.0f, 10000.0f), Mth::randRange(-10000.0f, 10000.0f)));
        std::cout << "RANDOMIZED WORLD SEED" << std::endl;
    }
    // recompile shaders if prompted
    if (Input::justPressed(sf::Keyboard::Num0)) {
        Resources::get().buildShaders();
    }
    if (Input::justPressed(sf::Keyboard::Tab) && gameFocused) { // gameFocused cuz alt tab
        paused = !paused;
    }
    if (Input::justPressed(sf::Keyboard::F1)) {
        showFPS = !showFPS;
    }
}

void Game::update(GLfloat delta) {
    // plus and minus keys are on shift layer so use equals and dash for convenience
    if (Input::pressed(sf::Keyboard::Equal)) {
        mouseScroll += 5.0f * delta;
    }
    if (Input::pressed(sf::Keyboard::Dash)) {
        mouseScroll -= 5.0f * delta;
    }

    // update menu
    running = menu->update(delta);

    if (menu->justClosed) {
        entityManager->init(NUM_ENEMIES);
        player->spawn(glm::vec3(0.0f, SPAWN_HEIGHT - SPAWN_OFFSET, 0.0f), true);
        //Resources::get().menuTrack.stop();
        //Resources::get().mainTrack.play();
    }
    if (menu->justOpened) {
        player->spawn(glm::vec3(0.0f, SPAWN_HEIGHT - SPAWN_OFFSET, 0.0f), false);
        entityManager->returnAllObjects();
        //Resources::get().menuTrack.play();
        //Resources::get().mainTrack.stop();
    }

    // update audio
    audio->update(delta);

    // update camera
    if (player->isDead) {
        cam.behavior = CameraMode::DEATH;
    } else if (menu->getVisible()) {
        cam.behavior = CameraMode::AUTOSPIN;
        player->spawn(glm::vec3(0.0f, SPAWN_HEIGHT - SPAWN_OFFSET, 0.0f), false);
    } else {
        cam.behavior = CameraMode::NORMAL;
    }
    cam.updateCameraDistance(mouseScroll);
    cam.update(mouseMove.x, mouseMove.y, delta);

    if (paused) {   // dont update main game stuff if paused
        return;
    }

    entityManager->update(delta);

    glm::vec3 pp = player->getTransform()->getWorldPos();
    //std::cout << pp.x << " " << pp.y << " " << pp.z << std::endl;
    terrain->update(pp);

    // detect and resolve collisions
    physics->update(delta, pp);

    //testMathUtils();

}

void Game::render() {
    if (wireframe) {    // maybe later make it only add in colliders near player
        int curDebugLen = physics->getColliderModels(*dmodels, *dcolors);
        graphics->setDebugStream(curDebugLen, dmodels, dcolors);
    } else {
        graphics->setDebugStream(0, dmodels, dcolors);
    }

    // render main game scene
    graphics->renderScene(cam, *terrain, blurring);

    // reset states to begin SFML 2D rendering
    window->resetGLStates();
    // draw UI
    menu->draw(*window, showFPS);

    // do final post processing of image (draws skybox here as well)
    graphics->finalProcessing(cam, blurring);

    // swap buffers
    window->display();
}

// test math utils by drawing a bunch of boxes using each function
void Game::testMathUtils() {
    int numTestBoxes = 500000;
    float size = 10.0f;
    float curTime = fmod(gameTime.getElapsedTime().asSeconds(), 20.0f);
    curTime = 0.0f;
    glm::vec3 scale = glm::vec3(1.0f);
    for (int i = 0; i < numTestBoxes; ++i) {
        glm::vec3 p;
        if (curTime < 5.0f) {
            p = Mth::randInsideUnitCube() * size;
        } else if (curTime < 10.0f) {
            p = Mth::randInsideSphere(size);
        } else if (curTime < 15.0f) {
            p = glm::vec3(Mth::randRange(-size, size), Mth::randRange(-size, size), Mth::randRange(-size, size));
        } else {
            p = glm::vec3(Mth::rand01()*size*2.0f - size, Mth::rand01()*size*2.0f - size, Mth::rand01()*size*2.0f - size);
        }

        p.y += size;
        glm::mat4 model;
        model = glm::translate(model, p);
        model = glm::scale(model, scale);

        HSBColor color(Mth::rand01(), 1.0f, 1.0f);
        Graphics::addToStream(true, model, color.toRGB());
        //Graphics::addToStream(true, model, glm::vec3(1.0f));
    }
}

// delete all our dumb pointers lol
Game::~Game() {
    delete window;
    delete graphics;
    delete physics;
    delete terrain;
    delete player;
    delete entityManager;
    delete menu;
    delete audio;

    delete dmodels;
    delete dcolors;
}