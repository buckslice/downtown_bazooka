#pragma once

#include "gameObject.h"
#include <iostream>

const int MAX_HEALTH = 100;
const int HEALTH_BAR_HEIGHT = 20;
class Player : public GameObject {
public:
	Player(float initialVelocity = 0.0f, float initialAcceleration = 0.0f, int initialHealth = MAX_HEALTH) {
		this->velocity = initialVelocity;
		this->acceleration = initialAcceleration;
		this->health = initialHealth;
		healthBar.setFillColor(sf::Color::Green);
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

	void changeHealth(int difference) {
		health -= difference;
		normalizeHealth();
	}

	void drawHealthBar(sf::RenderWindow& window) {
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

	void normalizeHealth() {
		if (health >= MAX_HEALTH) {
			health = MAX_HEALTH;
		}
		if (health <= 0) {
			health = 0;
		}
	}
};