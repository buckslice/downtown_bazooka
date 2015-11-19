#include "input.h"

static bool thisframe[KEYS];
static bool lastframe[KEYS];

Input::Input() {
    update();
}

void Input::update() {
    for (int i = 0; i < KEYS; i++) {
        lastframe[i] = thisframe[i];
        thisframe[i] = sf::Keyboard::isKeyPressed(static_cast<sf::Keyboard::Key>(i));
    }
}

bool Input::pressed(sf::Keyboard::Key key) {
    return thisframe[key];
}

bool Input::justPressed(sf::Keyboard::Key key) {
    return thisframe[key] && !lastframe[key];
}

bool Input::justReleased(sf::Keyboard::Key key) {
    return lastframe[key] && !thisframe[key];
}