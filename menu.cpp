#include "menu.h"
#include "input.h"
#include <iostream>

#define USESPRITES 0

Menu::Menu(Player* player) {
	menu = nullptr;

    curSelection = 0;

	sf::Texture image;
	setVisible(true);

#if USESPRITES
	bool b = image.loadFromFile("assets/images/ui/Title.png");
	titleSprite.setTexture(image);
#endif //USESPRITES
	
            //menu[0].setPosition(sf::Vector2f(width / 2 - 65.0f, height / (MAX_NUMBER_OF_ITEMS + 1) * 1.3f));
            //menu[1].setPosition(sf::Vector2f(width / 2 - 190.0f, height / (MAX_NUMBER_OF_ITEMS + 1) * 2.3f));
            //menu[2].setPosition(sf::Vector2f(width / 2 - 65.0f, height / (MAX_NUMBER_OF_ITEMS + 1) * 3.3f));

    /*menu[0].setFont(font);
    menu[0].setColor(SELECTED_COLOR);
    menu[0].setString("Play");
    menu[0].setScale(sf::Vector2f(2.0f, 2.0f));

    menu[1].setFont(font);
    menu[1].setColor(DEFAULT_COLOR);
    menu[1].setString("Instructions");
    menu[1].setScale(sf::Vector2f(2.0f, 2.0f));

    menu[2].setFont(font);
    menu[2].setColor(DEFAULT_COLOR);
    menu[2].setString("Quit");
    menu[2].setScale(sf::Vector2f(2.0f, 2.0f));*/

    title.setFont(Resources::get().font);
	title.setCharacterSize(90);
    title.setColor(TITLE_COLOR);
    title.setString("DOWNTOWN BAZOOKA");

    instructions.setFont(Resources::get().font);
    instructions.setColor(DEFAULT_COLOR);
    instructions.setString( // instead of adding a space to front should just move pos over lol XD
        "WASD : move\n"
        "E    : shoot\n"
        "Space: jump, select\n"
        "ESC  : menu, quit\n"
        "---------------------------\n"
        "TAB  : pause\n"
        "Q    : toggle flymode\n"
        "1    : toggle blur\n"
        "2    : toggle mipmaps\n"
        "3    : toggle physics debug\n"
        "4    : toggle terrain debug\n"
        "5    : randomize world seed\n"
        "6    : gen colorwheel\n"
        "7    : gen normal\n"
        "0    : recompile shaders");

    instructions.setScale(sf::Vector2f(1.5f, 1.5f));
    instructions.setPosition(5.0f, 50.0f);
    healthBar.setFillColor(sf::Color(180, 255, 0, 255));

    this->player = player;

	this->setVisible(true);

    // this should give the width in pixels of the text so we can use that to center it
    // but too bad it crashes the game so hard lol whyyyyyy???
    // some sort of weird access violation where we arent reseting states right??? bugged pos???
    //sf::FloatRect rect = title.getLocalBounds();


}

Menu::~Menu() {
}

void Menu::draw(sf::RenderWindow& window) {
    int width = window.getSize().x;
    int height = window.getSize().y;
    if (visible) {
        if (showingInstructions) {
            window.draw(instructions);
        } else {
			sf::RectangleShape shape;
			shape.setFillColor(sf::Color(0,0,0,200));
			shape.setPosition(sf::Vector2f());
			shape.setSize(sf::Vector2f(width,height));
			window.draw(shape);
            // set positions incase resize
#if USESPRITES
			titleSprite.setPosition(width/2,height/2);
			window.draw(titleSprite);
#endif //USESPRITES

            title.setPosition(sf::Vector2f(width / 2 - 440.0f, height / (MAX_NUMBER_OF_ITEMS + 1) * 0.3f));
            //menu[0].setPosition(sf::Vector2f(width / 2 - 65.0f, height / (MAX_NUMBER_OF_ITEMS + 1) * 1.3f));
            //menu[1].setPosition(sf::Vector2f(width / 2 - 190.0f, height / (MAX_NUMBER_OF_ITEMS + 1) * 2.3f));
            //menu[2].setPosition(sf::Vector2f(width / 2 - 65.0f, height / (MAX_NUMBER_OF_ITEMS + 1) * 3.3f));

            window.draw(title);
			if(menu == nullptr)
				setVisible(true);
            for (int i = 0; i < MAX_NUMBER_OF_ITEMS; i++) {
				menu[i]->draw(window,sf::Vector2f(width/2,height/(MAX_NUMBER_OF_ITEMS+1)*(1.f+i)));
                //window.draw(menu[i]);
            }
        }
    } else {
        healthBar.setPosition(0.0f, static_cast<float>(height - HEALTH_BAR_HEIGHT));
        float x = static_cast<float>(width * player->getHealth() / MAX_HEALTH);
        float y = static_cast<float>(HEALTH_BAR_HEIGHT);
        healthBar.setSize(sf::Vector2f(x, y));
        //window.draw(healthBar);   // commenting for now until we get actual damage events
    }
}

int circularClamp(int n, int min, int max) {
    if (n < min) {
        return max;
    } else if (n > max) {
        return min;
    }
    return n;
}

void Menu::move(bool up) {
    menu[curSelection]->SetIsSelected(false);
    curSelection += up ? -1 : 1;
    curSelection = circularClamp(curSelection, 0, MAX_NUMBER_OF_ITEMS - 1);
    menu[curSelection]->SetIsSelected(true);
}


void Menu::update(bool& running) {
    justClosed = false;
    justOpened = false;
    if (Input::justPressed(sf::Keyboard::Escape)) {
        if (visible) {
            if (showingInstructions) {
                showingInstructions = false;
            } else {
                running = false;
            }
        } else {
            visible = true;
            justOpened = true;
        }
    }

    if (!visible) {
        return;
    }

    if (Input::justPressed(sf::Keyboard::W) || Input::justPressed(sf::Keyboard::Up)) {
        move(true);
    }

    if (Input::justPressed(sf::Keyboard::S) || Input::justPressed(sf::Keyboard::Down)) {
        move(false);
    }

    if (Input::justPressed(sf::Keyboard::Return) || Input::justPressed(sf::Keyboard::Space)) {
        switch (curSelection) {
        case 0:
            setVisible(false);
            justClosed = true;
            break;
        case 1:
            showingInstructions = true;
            break;
        case 2:
            running = false;
            break;
        }
    }
}

void Menu::setVisible(bool visible) {
	if (visible && menu == nullptr) {
		menu = new TextOption*[MAX_NUMBER_OF_ITEMS];
		menu[0] = new TextOption("Play");
		menu[1] = new TextOption("Instructions");
		menu[2] = new TextOption("Quit");
		menu[0]->SetIsSelected(true);
	}else if(!visible && menu != nullptr){
		for(int i = 0; i < MAX_NUMBER_OF_ITEMS; i++)
			delete menu[i];
		delete[] menu;
		menu = nullptr;
	}
    this->visible = visible;
}

bool Menu::getVisible() {
    return visible;
}
