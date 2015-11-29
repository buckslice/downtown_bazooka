#pragma once

#include <iostream>
#include <algorithm>
#include "entity.h"
#include "camera.h"
#include "physics.h"
#include "input.h"

const int MAX_HEALTH = 100;
const int HEALTH_BAR_HEIGHT = 20;

const GLfloat SPEED = 15.0f;
const GLfloat JUMPSPEED = 25.0f;
const GLfloat GRAVITY = -30.0f;


class Player : public Entity {
public:
    bool flying = false;

    Player(Camera* cam) : speed(SPEED) {
        this->cam = cam;
    }

    int getHealth();

    void update(GLfloat delta) override;

    void jump();

    void addHealth(int amount);

private:
    Camera* cam;
    int health;
    float speed;
    float timeSinceJump = -1.0f;

    glm::vec3 getMovementDir();
};