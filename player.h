#pragma once

#include <iostream>
#include <algorithm>
#include "entity.h"
#include "camera.h"
#include "physics.h"
#include "input.h"
#include "projectile.h"

const float MAX_HEALTH = 100.0f;
const int HEALTH_BAR_HEIGHT = 20;

const GLfloat SPEED = 15.0f;
const GLfloat JUMPSPEED = 25.0f;
const GLfloat SHOTS_PER_SECOND = 1.0f;
const GLfloat SHOOT_SPEED = 25.0f;

//extern Player* PlayerInstance; // extern singleton for the player class

class Player : public Entity {
public:
    bool flying = false;
	bool isDead = false;

    Player(Camera* cam);

	void spawn(glm::vec3 spawnPos, bool awake);

    float getHealth();
	float getMaxHealth();
	float getDamage();

    void update(GLfloat delta) override;
    void onCollision(CollisionData data) override;

    void jump();

    void shoot();

    void addHealth(float amount);

private:
    Camera* cam;
	float health, maxHealth, damage, jumpSpeed, shotsPerSecond, shootSpeed,
		invulnTime = 0.0f, speed, timeSinceJump = -1.0f, timeSinceShot = -1.0f;

    glm::quat currRot, targRot;

    glm::vec3 getMovementDir();
};