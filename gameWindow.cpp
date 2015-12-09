#include "gameWindow.h"

GameWindow::GameWindow(int width, int height)
	: width{width}, height{height}, running{true},
	lastFocused{false}, gameFocused{false}, clickedInside{true}, mouseVisible{false}, lastMouseVisible{false}
{
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


	// init random seed and build city
	srand(time(NULL));
	//srand(1); //my testing seed for physics bugs lol


	// generate a random city
	cg->generate(false, false, 7500, *physics);

	// init camera
	player = new Player(&cam);
	cam.transform.parent = player->transform;
	cam.transform.pos = glm::vec3(0.0f, 1.8f, 0.0f);

	em = new EntityManager(player);

	menu = new Menu(width, height, player);

	// set up music
	mainTrack.setLoop(true);
	mainTrack.setVolume(20.0f);
	mainTrack.play();

	// mouse and window focusing variables
	sf::Mouse::setPosition(center, *window);
	window->setMouseCursorVisible(false);
	window->setKeyRepeatEnabled(false); // so sf::Event::KeyPressed will be useful

}


GameWindow::~GameWindow()
{
    delete window;
    delete graphics;
    delete physics;
    delete cg;
    delete player;
    delete em;
    delete menu;
}

void GameWindow::mainLoop()
{
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
				graphics->resize(e.size.width, e.size.height);
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
			mouseMove = input.getMouseMovement(*window, !lastFocused, center);
		}

		// update static input bool arrays
		if (gameFocused || lastFocused) {
			input.update();
		}

		// recompile shaders if prompted
		if (Input::justPressed(sf::Keyboard::R)) {
			graphics->buildShaders();
		}

		// build square or circular city if prompted
		if (Input::justPressed(sf::Keyboard::F) || Input::justPressed(sf::Keyboard::G)) {
			physics->clearStatics();
			cg->generate(false, Input::justPressed(sf::Keyboard::F), 7500, *physics);
		}

		//update menu
		menu->update(running);

		if (menu->justClosed) {
			player->flying = false;
			em->init(2000);
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
		}
		else {
			// reset player to floating above city
			player->setPosition(glm::vec3(0.0f, 200.0f, 0.0f));
			player->getTransform()->vel = glm::vec3(0.0f);
			// auto scroll camera
			cam.pitch = 0.0f;
			cam.yaw += 5.0f * delta;
			cam.updateCameraVectors();
		}

		// resolve collisions
		physics->update(delta);

		// render graphics
		Graphics::setMeshVisible(em->dudeMesh, !menu->getVisible());
		graphics->renderScene(cam);

		// draw UI
		window->resetGLStates();
		menu->draw(*window);

		// apply bloom
		graphics->postProcess();

		// swap buffers
		window->display();
	}
}

bool GameWindow::isRunning()
{
	return running;
}
