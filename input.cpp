#include "input.h"

static bool thisframe[KEYS];
static bool lastframe[KEYS];

static bool bthisframe[BUTTONS];
static bool blastframe[BUTTONS];

Input::Input() {
    update();
}

void Input::update() {
    for (int i = 0; i < KEYS; i++) {
        lastframe[i] = thisframe[i];
        thisframe[i] = sf::Keyboard::isKeyPressed(static_cast<sf::Keyboard::Key>(i));
    }
    for (int i = 0; i < BUTTONS; i++) {
        blastframe[i] = bthisframe[i];
        bthisframe[i] = sf::Mouse::isButtonPressed(static_cast<sf::Mouse::Button>(i));
    }
}

void Input::forgetLeftClickThisFrame() {
    blastframe[sf::Mouse::Left] = true;
}

bool Input::pressed(sf::Keyboard::Key key) {
    return thisframe[key];
}
bool Input::justPressed(sf::Keyboard::Key key) {
    return thisframe[key] && !lastframe[key];
}
bool Input::justReleased(sf::Keyboard::Key key) {
    return !thisframe[key] && lastframe[key];
}

bool Input::pressed(sf::Mouse::Button button) {
    return bthisframe[button];
}
bool Input::justPressed(sf::Mouse::Button button) {
    return bthisframe[button] && !blastframe[button];
}
bool Input::justReleased(sf::Mouse::Button button) {
    return !bthisframe[button] && blastframe[button];
}

sf::Vector2i Input::getMouseMovement(sf::Window& window, bool centerAndIgnore, sf::Vector2i center) {
    sf::Vector2i mouseMove;

    // if window just got refocused or on resize then recenter mouse and ignore
    if (centerAndIgnore) {
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
    return mouseMove;
}


