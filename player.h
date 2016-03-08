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

const GLfloat SPEED = 15.0f;
const GLfloat JUMPSPEED = 25.0f;

class Player : public Entity {
public:
    bool flying = false;
	bool isDead = false;

    Player(Camera* cam);

	void spawn(glm::vec3 spawnPos);

    int getHealth();

    void update(GLfloat delta) override;
    void onCollision(Collider* other) override;

    void jump();

    void shoot();

    void addHealth(int amount);

private:
    Camera* cam;
    int health;
	float invulnTime = 0.0f;
    float speed;
    float timeSinceJump = -1.0f;

    glm::quat currRot;
    glm::quat targRot;

    glm::vec3 getMovementDir();
};