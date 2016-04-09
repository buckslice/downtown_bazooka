#pragma once

#include <iostream>
#include <algorithm>
#include "entity.h"
#include "camera.h"
#include "physics.h"
#include "input.h"
#include "projectile.h"

const int HEALTH_BAR_HEIGHT = 20;

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

    void addHealth(float amount);

private:
    void jump();
    void shoot();

    Camera* cam;
    float speed = 15.0f;
    float maxHealth = 100.0f;
    float health = maxHealth;
    float damage = 10.0f;
    float jumpSpeed = 25.0f;
    float shotsPerSecond = 1.0f;
    float shootSpeed = 25.0f;
    float invulnTime = 0.0f;
    float timeSinceHitJump = -1.0f;
    float timeSinceShot = -1.0f;

    glm::quat currRot, targRot;

    glm::vec3 getMovementDir();
};