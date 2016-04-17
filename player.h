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
    void onCollision(Tag tag, Entity* other) override;

    void addHealth(float amount);

private:
    bool getGroundedRecent();
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
    float timeSinceHitJump = 10.0f;
    float timeSinceShot = 10.0f;
    float timeSinceGrounded = 10.0f;
    float burnTime = 0.0f;
    // allows player to still jump if they try a little before they are allowed
    const float jumpLenience = 0.2f;
    // allows player to count as grounded potentially a little after they actually are
    const float groundedLenience = 0.25f;   

    glm::quat currRot, targRot;

    glm::vec3 getMovementDir();
};