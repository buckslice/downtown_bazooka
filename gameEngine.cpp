#include "gameEngine.h"

GameEngine::GameEngine(GLuint width, GLuint height)
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
    cam.transform->setPos(glm::vec3(0.0f, 1.8f, 0.0f));
    player->transform->parentAll(cam.transform);
    player->spawn(glm::vec3(0.0f, 150.0f, 0.0f), false);

    entityManager = new EntityManager(player);

    menu = new Menu(player);
    window->resetGLStates();

    audio = new Audio();
    Resources::get().mainTrack.play();

    // mouse and window focusing variables
    sf::Mouse::setPosition(center, *window);
    window->setMouseCursorVisible(false);
    window->setKeyRepeatEnabled(false); // so sf::Event::KeyPressed will be more accurate

    // initiate game helper class
    game = new Game(player->transform);
}

void GameEngine::start() {
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

void GameEngine::pollEvents() {
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

void GameEngine::getInput() {
    // check and track game focusing
    lastFocused = gameFocused;
    gameFocused = window->hasFocus() && clickedInside;

    // update static input bool arrays
    if (gameFocused || lastFocused) {
        input.update();
    }

    if (gameFocused && !lastFocused) {
        input.forgetLeftClickThisFrame();
    }

    // check if clicked inside window this frame
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
}


void GameEngine::toggleOptions() {
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
        std::cout << "PHYSICS DEBUG " << (wireframe ? "ON" : "OFF") << std::endl;
    }
    // toggle terrain color debug
    if (Input::justPressed(sf::Keyboard::Num4)) {
        regenerateWorld();
        std::cout << "TERRAIN DEBUG " << (terrain->toggleDebugColors() ? "ON" : "OFF") << std::endl;
    }
    // randomize world seed
    if (Input::justPressed(sf::Keyboard::Num5)) {
        regenerateWorld();
        terrain->setSeed(glm::vec2(Mth::randRange(-10000.0f, 10000.0f), Mth::randRange(-10000.0f, 10000.0f)));
        std::cout << "RANDOMIZED WORLD SEED" << std::endl;
    }
    // recompile shaders if prompted
    if (Input::justPressed(sf::Keyboard::Num0)) {
        Resources::get().buildShaders();
    }
    if (Input::justPressed(sf::Keyboard::Tab) && gameFocused) { // && gameFocused cuz alt tab
        paused = !paused;
    }
    if (Input::justPressed(sf::Keyboard::F1)) {
        showFPS = !showFPS;
    }
}

void GameEngine::update(GLfloat delta) {
    // plus and minus keys are on shift layer so use equals and dash for convenience
    if (Input::pressed(sf::Keyboard::Equal)) {
        mouseScroll += 5.0f * delta;
    }
    if (Input::pressed(sf::Keyboard::Dash)) {
        mouseScroll -= 5.0f * delta;
    }

    // update menu
    running = menu->update(delta);

    if (Menu::justClosed()) {
        player->spawn(glm::vec3(0.0f, 5.0f, 0.0f), true);
    }
    if (Menu::justOpened()) {
        game->reset();
        regenerateWorld();
        player->spawn(glm::vec3(0.0f, 150.0f, 0.0f), false);
    }

    // update camera
    cam.behavior = player->isDead() ? CameraMode::DEATH :
        menu->getVisible() ? CameraMode::AUTOSPIN : CameraMode::NORMAL;
    cam.updateCameraDistance(mouseScroll);
    cam.update(mouseMove.x, mouseMove.y, delta);

    if (paused) {   // dont update main game stuff if paused
        return;
    }

    // update game first because it stores delta time and other useful variables
    game->update(delta);
    if (Game::requiresWorldRegen()) {
        regenerateWorld();
        Game::setRequiresWorldRegen(false);
    }

    // update audio
    audio->update(delta);

    // update all game entities
    entityManager->update(delta);

    terrain->update(delta);

    // detect and resolve collisions
    physics->update(delta);

}

void GameEngine::render() {
    if (wireframe) {    // maybe later make it only add in colliders near player
        Graphics::DEBUG = true;
        physics->streamColliderModels();
    } else {
        Graphics::DEBUG = false;
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

void GameEngine::regenerateWorld() {
    terrain->deleteChunks();
    physics->rebuildCollisionTree(0.0f);
    entityManager->returnAllObjects();
}

// test math utils by drawing a bunch of boxes using each function
void GameEngine::testMathUtils() {
    int numTestBoxes = 500000;
    float size = 10.0f;
    float curTime = fmod(Game::timeSinceStart(), 20.0f);
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
        Graphics::addToStream(Shape::CUBE_SOLID, model, color.toRGB());
        //Graphics::addToStream(true, model, glm::vec3(1.0f));
    }
}

// delete all our dumb pointers lol
GameEngine::~GameEngine() {
    delete window;
    delete graphics;
    delete physics;
    delete terrain;
    delete player;
    delete entityManager;
    delete menu;
    delete audio;
    delete game;
}