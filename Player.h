#pragma once

#include "gameObject.h"
#include <iostream>
#include <algorithm>

const int MAX_HEALTH = 100;
const int HEALTH_BAR_HEIGHT = 20;
class Player : public GameObject {
public:
    Player(float initialVelocity = 0.0f, float initialAcceleration = 0.0f, int initialHealth = MAX_HEALTH) {
        this->velocity = initialVelocity;
        this->acceleration = initialAcceleration;
        this->health = initialHealth;
        //healthBar.setFillColor(sf::Color::Green);
        healthBar.setFillColor(sf::Color(180, 255, 0, 255));
    }

    float getVelocity() {
        return velocity;
    }

    float getAcceleration() {
        return acceleration;
    }

    int getHealth() {
        return health;
    }

    void setVelocity(float newVelocity) {
        velocity = newVelocity;
    }

    void setAcceleration(float newAcceleration) {
        acceleration = newAcceleration;
    }

    void addHealth(int amount) {
        health += amount;
        health = std::max(0, std::min(health, MAX_HEALTH));
    }

    void drawHealthBar(sf::RenderWindow& window) {
        healthBar.setPosition(0.0f, window.getSize().y - HEALTH_BAR_HEIGHT);
        healthBar.setSize(sf::Vector2f(window.getSize().x * health / MAX_HEALTH, HEALTH_BAR_HEIGHT));
        window.draw(healthBar);
    }

    void update(sf::RenderWindow& window) {
        drawHealthBar(window);

    }

private:
    float velocity;
    float acceleration;
    int health;
    sf::RectangleShape healthBar;
};