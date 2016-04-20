#include "menu.h"
#include "input.h"
#include <iostream>
#include <iomanip>
#include "audio.h"

#define USESPRITES 1

Menu::Menu(Player* player) {
    menu = nullptr;
    curSelection = 0;
    this->player = player;
    setVisible(true);

    //if (titleTexture.loadFromFile("assets/images/ui/Title.png")) {
    //    titleSprite.setTexture(titleTexture);
    //}

    title.setFont(Resources::get().font);
    title.setCharacterSize(110U);
    title.setColor(TITLE_COLOR);
    title.setString("DOWNTOWN BAZOOKA");

    instructions.setFont(Resources::get().font);
    instructions.setColor(SELECTED_COLOR);
    instructions.setString( // instead of adding a space to front should just move pos over lol XD
        "WASD : move\n"
        "E    : shoot\n"
        "Space: jump, select\n"
        "ESC  : menu, quit\n"
        "---------------------------\n"
        "TAB  : pause\n"
        "F1   : show FPS\n"
        "Q    : toggle flymode\n"
        "1    : toggle blur\n"
        "2    : toggle mipmaps\n"
        "3    : toggle physics debug\n"
        "4    : toggle terrain debug\n"
        "5    : randomize world seed\n"
        "0    : recompile shaders");

    instructions.setScale(sf::Vector2f(1.5f, 1.5f));
    instructions.setPosition(5.0f, 50.0f);
    healthBar.setFillColor(sf::Color(180, 255, 0, 255));

    fpsText.setFont(Resources::get().font);
    fpsText.setColor(sf::Color(255, 0, 0));
    fpsText.setScale(2.0f, 2.0f);
    fpsText.setPosition(0.0f, -10.0f);
    deadText.setFont(Resources::get().font);
    deadText.setColor(sf::Color(255, 0, 0));
    deadText.setScale(4.0f, 4.0f);
    deadText.setString("GAME OVER!");

    overlay.setFillColor(sf::Color(0, 0, 0, 150));
    overlay.setPosition(sf::Vector2f());


    // this should give the width in pixels of the text so we can use that to center it
    // but too bad it crashes the game so hard lol whyyyyyy???
    // some sort of weird access violation where we arent reseting states right??? bugged pos???
    //sf::FloatRect rect = title.getLocalBounds();

}

Menu::~Menu() {
}

void Menu::draw(sf::RenderWindow& window, bool showFPS) {
    int width = window.getSize().x;
    int height = window.getSize().y;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.1f);

    if (showFPS) {
        window.draw(fpsText);
    }

    // if main menu is not being shown then draw game UI stuff
    if (!visible) {
        if (player->isDead()) {
            overlay.setSize(sf::Vector2f(sf::Vector2i(width, height)));
            glDepthMask(GL_FALSE);
            window.draw(overlay);
            glDepthMask(GL_TRUE);

            deadText.setPosition(width / 2.0f - 325.0f, height / 5.0f);
            window.draw(deadText);
        } else {
            healthBar.setPosition(0.0f, static_cast<float>(height - HEALTH_BAR_HEIGHT));
            float x = static_cast<float>(width * player->getHealth() / player->getMaxHealth());
            float y = static_cast<float>(HEALTH_BAR_HEIGHT);
            healthBar.setSize(sf::Vector2f(x, y));
            window.draw(healthBar);
        }
        return;
    }

    overlay.setSize(sf::Vector2f(sf::Vector2i(width, height)));
    glDepthMask(GL_FALSE);
    window.draw(overlay);
    glDepthMask(GL_TRUE);

    if (showingInstructions) {
        window.draw(instructions);
    } else {
        //titleSprite.setPosition(width / 2.0f - 500.0f, height / 5.0f);
        //window.draw(titleSprite);

        title.setPosition(sf::Vector2f(width / 2.0f - 530.0f, height * 0.05f));
        window.draw(title);

        if (menu == nullptr) {
            setVisible(true);
        }
        for (int i = 0; i < MAX_NUMBER_OF_ITEMS; i++) {
            menu[i]->draw(window, sf::Vector2f(width / 2.0f, height / (MAX_NUMBER_OF_ITEMS + 1)*(1.0f + i)));
            //window.draw(menu[i]);
        }
    }

    glDisable(GL_ALPHA_TEST);
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
	AudioInstance->playSound(Resources::get().menuMoveSound);
    menu[curSelection]->SetIsSelected(false);
    curSelection += up ? -1 : 1;
    curSelection = circularClamp(curSelection, 0, MAX_NUMBER_OF_ITEMS - 1);
    menu[curSelection]->SetIsSelected(true);
}


bool Menu::update(GLfloat delta) {

    updateFpsText(delta);

    justClosed = false;
    justOpened = false;
    if (Input::justPressed(sf::Keyboard::Escape)) {
        if (visible) {
            if (showingInstructions) {
                showingInstructions = false;
            } else {
                return false;
            }
        } else {
            visible = true;
            justOpened = true;
        }
    }

    if (!visible) {
        return true;
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
			AudioInstance->playSound(Resources::get().menuSelectSound);
            setVisible(false);
            justClosed = true;
            break;
        case 1:
			AudioInstance->playSound(Resources::get().menuSelectSound);
            showingInstructions = true;
            break;
        case 2:
            return false;
        }
    }
    return true;
}

void Menu::setVisible(bool visible) {
    if (visible && menu == nullptr) {
        menu = new TextOption*[MAX_NUMBER_OF_ITEMS];
        menu[0] = new TextOption("Play");
        menu[1] = new TextOption("Instructions");
        menu[2] = new TextOption("Quit");
        menu[0]->SetIsSelected(true);
    } else if (!visible && menu != nullptr) {
        for (int i = 0; i < MAX_NUMBER_OF_ITEMS; i++)
            delete menu[i];
        delete[] menu;
        menu = nullptr;
    }
    this->visible = visible;
}

// calculates a buffers the fps
void Menu::updateFpsText(float delta) {
    if (delta < 0.005f) {
        delta = 0.005f;
    }
    float fps = 1.0f / delta;
    fpsValues.push(fps);
    totalFpsQueueValue += fps;
    while (fpsValues.size() > 30) {
        totalFpsQueueValue -= fpsValues.front();
        fpsValues.pop();
    }
    float avgFps = totalFpsQueueValue / fpsValues.size();
    std::stringstream ss;
    ss << std::fixed << std::setprecision(1) << avgFps;
    fpsText.setString(ss.str());
}

bool Menu::getVisible() {
    return visible;
}
