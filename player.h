#pragma once

#include <iostream>
#include <algorithm>
#include "entity.h"
#include "camera.h"
#include "physics.h"
#include "input.h"
#include "projectile.h"

const int MAX_HEALTH = 100;
const int HEALTH_BAR_HEIGHT = 20;

const GLfloat SPEED = 5.0f;
const GLfloat JUMPSPEED = 15.0f;

class Player : public Entity {
public:
    bool flying = false;

    Player(Camera* cam) : speed(SPEED) {
        this->cam = cam;
        timeSinceJump = 10.0f;
    }

    int getHealth();

    void update(GLfloat delta) override;

    void jump();

	void shoot();

    void addHealth(int amount);

	std::vector<Projectile> getProjectiles() const;

private:
    Camera* cam;
    int health;
	std::vector<Projectile> projectiles;
    float speed;
    float timeSinceJump = -1.0f;

    glm::vec3 getMovementDir();
};