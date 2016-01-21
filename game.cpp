#include "game.h"

Game::Game(int width, int height)
    : width{ width }, height{ height },
    running{ true }, lastFocused{ false }, gameFocused{ false },
    clickedInside{ true }, mouseVisible{ false }, lastMouseVisible{ false },
    mipmapping{ true }, blurring{ true } {

    center.x = width / 2;
    center.y = height / 2;
    // build window
    sf::ContextSettings settings;
    settings.depthBits = 24;
    settings.stencilBits = 8;
    settings.antialiasingLevel = 2;
    window = new sf::RenderWindow(sf::VideoMode(width, height), "DOWNTOWN BAZOOKA", sf::Style::Default, settings);
    window->setFramerateLimit(60);

    // load music track	
    if (!mainTrack.openFromFile("assets/music/expl1.ogg")) {
        std::cout << "ERROR::MUSIC_LOAD_FAILURE" << std::endl;
    }

    // main systems
    graphics = new Graphics(*window);
    Resources::get();   // load resources
    physics = new Physics();
    cg = new CityGenerator();
    tg = new TerrainGenerator();
    physics->tg = tg;

    // init random seed and build city
    srand(static_cast<unsigned int>(time(NULL)));
    //srand(1); //my testing seed for physics bugs lol


    // generate a random city
    cg->generate(false, true, 7500, *physics);

    // init camera
    player = new Player(&cam);
    cam.transform.parent = player->transform;
    cam.transform.lpos = glm::vec3(0.0f, 1.8f, 0.0f);

    em = new EntityManager(player);

    menu = new Menu(player);

    // set up music
    mainTrack.setLoop(true);
    mainTrack.setVolume(20.0f);
    //mainTrack.play(); // ENSIFERUM

    // mouse and window focusing variables
    sf::Mouse::setPosition(center, *window);
    window->setMouseCursorVisible(false);
    window->setKeyRepeatEnabled(false); // so sf::Event::KeyPressed will be useful

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
}

void Game::mainLoop() {
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
            {   // need these brackets
                float w = static_cast<float>(e.size.width);
                float h = static_cast<float>(e.size.height);
                window->setView(sf::View(sf::FloatRect(0.0f, 0.0f, w, h)));
                graphics->resize(e.size.width, e.size.height);
                center.x = e.size.width / 2;
                center.y = e.size.height / 2;
                break;
            }
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
            mouseMove = input.getMouseMovement(*window, !lastFocused, center);
        }

        // update static input bool arrays
        if (gameFocused || lastFocused) {
            input.update();
        }

        // rebuild color wheel city
        if (Input::justPressed(sf::Keyboard::F)) {
            physics->clearStatics();
            cg->generate(false, true, 7500, *physics);
            std::cout << ("BUILT COLOR WHEEL CITY") << std::endl;
        }
        // rebuild regular city
        if (Input::justPressed(sf::Keyboard::G)) {
            physics->clearStatics();
            cg->generate(false, false, 7500, *physics);
            std::cout << ("BUILT NORMAL CITY") << std::endl;
        }
        // randomize terrain
        if (Input::justPressed(sf::Keyboard::V)) {
            tg->deleteChunks();
            tg->setSeed(glm::vec2(Mth::randRange(-10000.0f, 10000.0f), Mth::randRange(-10000.0f, 10000.0f)));
            std::cout << "RANDOMIZED TERRAIN" << std::endl;
        }
        // toggle terrain color
        if (Input::justPressed(sf::Keyboard::B)) {
            tg->deleteChunks();
            tg->toggleDebugColors();
            std::cout << "TOGGLED TERRAIN COLORS" << std::endl;
        }
        // toggle mipmaps
        if (Input::justPressed(sf::Keyboard::R)) {
            mipmapping = !mipmapping;
            Resources::get().loadTextures(mipmapping);
            std::cout << "MIPMAPS " << (mipmapping ? "ON" : "OFF") << std::endl;
        }
        // toggle blur
        if (Input::justPressed(sf::Keyboard::T)) {
            blurring = !blurring;
            std::cout << "BLUR " << (blurring ? "ON" : "OFF") << std::endl;
        }
        // recompile shaders if prompted
        if (Input::justPressed(sf::Keyboard::Y)) {
            Resources::get().buildShaders();
        }

        //update menu
        menu->update(running);

        if (menu->justClosed) {
            player->flying = false;
            em->init(0);
        }
        if (menu->justOpened) {
            em->deleteEntities();
            physics->clearDynamics();
        }

        // update camera
        if (!menu->getVisible()) {   // if game running
            em->update(delta);

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

        tg->update(player->getTransform()->getPos());

        // resolve collisions
        physics->update(delta);

        // render graphics
        Graphics::setMeshVisible(em->dudeMesh, false);
        //Graphics::setMeshVisible(em->dudeMesh, !menu->getVisible());
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
}

bool Game::isRunning() {
    return running;
}
