#include "menu.h"
#include <iostream>



Menu::Menu(float width, float height)
{
	if (!font.loadFromFile("assets/fonts/MAGNETOB.ttf")) {
		//handle error
		std::cout << "ERROR::FONT::LOAD_FAILURE";
	}
	selectedItemIndex = 0;
	lastUp = lastDown = false;

	menu[0].setFont(font);
	menu[0].setColor(sf::Color::Red);
	menu[0].setString("Play");
	menu[0].setPosition(sf::Vector2f(width / 2 - 100.0f, height / (MAX_NUMBER_OF_ITEMS + 1) * 1));
	menu[0].setScale(sf::Vector2f(2.0f, 2.0f));

	menu[1].setFont(font);
	menu[1].setColor(sf::Color::White);
	menu[1].setString("Instructions");
	menu[1].setPosition(sf::Vector2f(width / 2 - 200.0f, height / (MAX_NUMBER_OF_ITEMS + 1) * 2));
	menu[1].setScale(sf::Vector2f(2.0f, 2.0f));

	menu[2].setFont(font);
	menu[2].setColor(sf::Color::White);
	menu[2].setString("Quit");
	menu[2].setPosition(sf::Vector2f(width / 2 - 100.0f, height / (MAX_NUMBER_OF_ITEMS + 1) * 3));
	menu[2].setScale(sf::Vector2f(2.0f, 2.0f));

	title.setFont(font);
	title.setColor(sf::Color::White);
	title.setString("DOWTOWN BAZOOKA");
	title.setPosition(sf::Vector2f(125.0f, height / (MAX_NUMBER_OF_ITEMS + 1) * 0.1));
	title.setScale(sf::Vector2f(3.0f, 3.0f));
}


Menu::~Menu()
{
}

void Menu::draw(sf::RenderWindow& window) {
	if (visible) {
		window.draw(title);
		for (int i = 0; i < MAX_NUMBER_OF_ITEMS; i++) {
			window.draw(menu[i]);
		}
	}
}

void Menu::moveUp() {
	menu[selectedItemIndex].setColor(sf::Color::White);
	if (selectedItemIndex > 0) {
		selectedItemIndex--;
	}
	else {
		selectedItemIndex = MAX_NUMBER_OF_ITEMS - 1;
	}
	menu[selectedItemIndex].setColor(sf::Color::Red);
}

void Menu::setVisible(bool visible) {
	this->visible = visible;
}

void Menu::showInstructions() {
	//TODO
}

void Menu::moveDown() {
	menu[selectedItemIndex].setColor(sf::Color::White);
	if (selectedItemIndex < MAX_NUMBER_OF_ITEMS - 1) {
		selectedItemIndex++;
	}
	else {
		selectedItemIndex = 0;
	}
	menu[selectedItemIndex].setColor(sf::Color::Red);
}

void Menu::executeItem(sf::RenderWindow& window, bool& running, Game& game) {
	switch (selectedItemIndex) {
	case 0:
		setVisible(false);
		game.start();
		break;
	case 1:
		showInstructions();
		break;
	case 2:
		running = false;
		window.close();
		break;
	}
}

void Menu::update(sf::RenderWindow& window, bool& running, Game& game) {
	draw(window);
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
		if (!lastUp) {
			moveUp();
			lastUp = true;
		}
	}
	else {
		lastUp = false;
	}

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
		if (!lastDown) {
			moveDown();
			lastDown = true;
		}
	}
	else {
		lastDown = false;
	}

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Return)) {
		executeItem(window, running, game);
	}
}