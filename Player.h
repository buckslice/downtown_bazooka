#pragma once

#include "gameObject.h"

class Player : public GameObject {
public:
	Player(float initialVelocity = 0.0f, float initialAcceleration = 0.0f, int initialHealth = 100) {
		this->velocity = initialVelocity;
		this->acceleration = initialAcceleration;
		this->health = initialHealth;
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
	}

private:
	float velocity;
	float acceleration;
	int health;
};