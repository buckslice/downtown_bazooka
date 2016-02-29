#include "game.h"

int numBuildings = 1000;   // 7500

Game::Game(GLuint width, GLuint height)
    : WIDTH{ width }, HEIGHT{ height },
    running{ true }, lastFocused{ false }, gameFocused{ false },
    clickedInside{ true }, mouseVisible{ false }, lastMouseVisible{ false },
    mipmapping{ true }, blurring{ true }, wireframe{ false }, paused{ false } {

    center.x = WIDTH / 2;
    center.y = HEIGHT / 2;
    // build window
    sf::ContextSettings settings;
    settings.depthBits = 24;
    settings.stencilBits = 8;
    settings.antialiasingLevel = 2;
    window = new sf::RenderWindow(sf::VideoMode(WIDTH, HEIGHT), "DOWNTOWN BAZOOKA", sf::Style::Default, settings);
    window->setFramerateLimit(60);

    // load music track	
    //if (!mainTrack.openFromFile("assets/music/expl1.ogg")) {
    //    std::cout << "ERROR::MUSIC_LOAD_FAILURE" << std::endl;
    //}
    //// set up music
    //mainTrack.setLoop(true);
    //mainTrack.setVolume(20.0f);
    //mainTrack.play(); // ENSIFERUM

    // main systems
    graphics = new Graphics(*window);
    Resources::get();   // load resources
    physics = new Physics();
    cg = new CityGenerator();
    tg = new Terrain();
    physics->tg = tg;

    // init random seed and build city
    srand(static_cast<unsigned int>(time(NULL)));   //TODO redo random usage with C++11!
    //srand(1); //my testing seed for physics bugs lol

    // generate a random city
    cg->generate(false, true, numBuildings, *physics);

    // init camera
    player = new Player(&cam);
    auto pt = player->getTransform();
    cam.transform->setPos(glm::vec3(0.0f, 1.8f, 0.0f));
    pt->parentAll(cam.transform);

    em = new EntityManager(player);

    menu = new Menu(player);

    // mouse and window focusing variables
    sf::Mouse::setPosition(center, *window);
    window->setMouseCursorVisible(false);
    window->setKeyRepeatEnabled(false); // so sf::Event::KeyPressed will be useful

    const GLuint MAX_DEBUG = 20000;
    dmodels = new std::vector<glm::mat4>(MAX_DEBUG);
    dcolors = new std::vector<glm::vec3>(MAX_DEBUG);
}

void Game::start() {
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
        tg->deleteChunks();
        std::cout << "TERRAIN DEBUG " << (tg->toggleDebugColors() ? "ON" : "OFF") << std::endl;
    }
    // randomize world seed
    if (Input::justPressed(sf::Keyboard::Num5)) {
        tg->deleteChunks();
        tg->setSeed(glm::vec2(Mth::randRange(-10000.0f, 10000.0f), Mth::randRange(-10000.0f, 10000.0f)));
        std::cout << "RANDOMIZED WORLD SEED" << std::endl;
    }
    // rebuild color wheel city
    if (Input::justPressed(sf::Keyboard::Num6)) {
        physics->clearStatics();
        cg->generate(false, true, numBuildings, *physics);
        std::cout << ("BUILT COLOR WHEEL CITY") << std::endl;
    }
    // rebuild regular city
    if (Input::justPressed(sf::Keyboard::Num7)) {
        physics->clearStatics();
        cg->generate(false, false, numBuildings, *physics);
        std::cout << ("BUILT NORMAL CITY") << std::endl;
    }
    // recompile shaders if prompted
    if (Input::justPressed(sf::Keyboard::Num0)) {
        Resources::get().buildShaders();
    }
    if (Input::justPressed(sf::Keyboard::Tab) && gameFocused) {
        paused = !paused;
    }
}

void Game::update(GLfloat delta) {

    //update menu
    menu->update(running);

    if (menu->justClosed) {
        player->flying = false;
        em->init(1000);
    }
    if (menu->justOpened) {
        em->returnAllObjects();
    }

    // update camera
    cam.setAutoSpin(menu->getVisible());
    cam.updateCameraDistance(mouseScroll);
    cam.update(mouseMove.x, mouseMove.y, delta);

    if (paused) {   // dont update main game stuff if paused
        return;
    }

    em->update(delta);
    if (menu->getVisible()) {
        // reset player to floating above city
        player->getTransform()->setPos(glm::vec3(0.0f, 150.0f, 0.0f));
        player->getCollider()->vel = glm::vec3(0.0f);
    }

    glm::vec3 pp = player->getTransform()->getWorldPos();
    //std::cout << pp.x << " " << pp.y << " " << pp.z << std::endl;
    tg->update(pp);

    // resolve collisions
    physics->update(delta);
}

void Game::render() {
    if (wireframe) {    // maybe later make it only add in colliders near player
        int curDebugLen = physics->getColliderModels(*dmodels, *dcolors);
        graphics->setDebugStream(curDebugLen, dmodels, dcolors);
    } else {
        graphics->setDebugStream(0, dmodels, dcolors);
    }

    // render graphics
    graphics->renderScene(cam, *tg, blurring);

    // draw UI
    window->resetGLStates();
    menu->draw(*window);

    if (blurring) {
        graphics->postProcess();
    }

    // swap buffers
    window->display();
}

// delete all our dumb pointers lol
Game::~Game() {
    delete window;
    delete graphics;
    delete physics;
    delete cg;
    delete tg;
    delete player;
    delete em;
    delete menu;

    delete dmodels;
    delete dcolors;
}