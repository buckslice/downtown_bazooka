#include "game.h"

int numBuildings = 7500;   // 7500

Game::Game(GLuint width, GLuint height)
    : WIDTH{ width }, HEIGHT{ height },
    running{ true }, lastFocused{ false }, gameFocused{ false },
    clickedInside{ true }, mouseVisible{ false }, lastMouseVisible{ false },
    mipmapping{ true }, blurring{ true }, wireframe{ false } {

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
    if (!mainTrack.openFromFile("assets/music/expl1.ogg")) {
        std::cout << "ERROR::MUSIC_LOAD_FAILURE" << std::endl;
    }

    // main systems
    graphics = new Graphics(*window);
    Resources::get();   // load resources
    physics = new Physics();
    cg = new CityGenerator();
    tg = new Terrain();
    physics->tg = tg;

    // init random seed and build city
    srand(static_cast<unsigned int>(time(NULL)));
    //srand(1); //my testing seed for physics bugs lol

    // generate a random city
    //cg->generate(false, true, numBuildings, *physics);

    // init camera
    player = new Player(&cam);
    auto pt = player->getTransform();
    pt->parentAll(cam.transform);
    cam.transform->setPos(glm::vec3(0.0f, 1.8f, 0.0f));

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
    const GLuint MAX_DEBUG = 20000;
    std::vector<glm::mat4> dmodels(MAX_DEBUG);
    std::vector<glm::vec3> dcolors(MAX_DEBUG);

    float avgfps = 0.0f;
    int fr = 0;

    while (running) {
        GLfloat delta = frameTime.restart().asSeconds();

        //fr++;
        //avgfps += 1.0f / delta;
        //std::cout << avgfps / fr << std::endl;

        // check for events that will quit game
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
                    cam.updateCameraDistance(-e.mouseWheelScroll.delta*2.0f, delta);
                }
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
            mouseMove = input.getMouseMovement(*window, !lastFocused, center);
        }

        // update static input bool arrays
        if (gameFocused || lastFocused) {
            input.update();
        }

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


        //update menu
        menu->update(running);

        if (menu->justClosed) {
            player->flying = false;
            em->init(0);
        }
        if (menu->justOpened) {
            em->deleteEntities();
            //physics->clearDynamics();
        }

        // update camera
        if (!menu->getVisible()) {   // if game running
            em->update(delta);
            cam.setAutoSpin(false);
        } else {
            // reset player to floating above city
            player->setPosition(glm::vec3(0.0f, 150.0f, 0.0f));
            player->getTransform()->vel = glm::vec3(0.0f);
            cam.setAutoSpin(true);
        }

        // update camera
        cam.update(mouseMove.x, mouseMove.y, delta);

        glm::vec3 pp = player->getTransform()->getWorldPos();
        //std::cout << pp.x << " " << pp.y << " " << pp.z << std::endl;
        tg->update(pp);

        // resolve collisions
        physics->update(delta);

        // update dudes model
        //glm::mat4 model;
        //glm::vec3 pos = player->getTransform()->lpos;
        //glm::vec3 scale = glm::vec3(1.0f, 2.0f, 1.0f);
        //pos.y += 1.0f;
        //model = glm::translate(model, pos);
        //model = glm::scale(model, scale);
        //Graphics::addToStream(false, model, glm::vec3(1.0f, 1.0f, 0.0f));

        if (wireframe) {    // maybe make it later only add in coliders near player
            int curDebugLen = physics->getColliderModels(dmodels, dcolors);
            graphics->setDebugStream(curDebugLen, &dmodels, &dcolors);
        } else {
            graphics->setDebugStream(0, &dmodels, &dcolors);
        }

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
