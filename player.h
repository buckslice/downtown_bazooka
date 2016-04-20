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
    bool flyMode = false;

    Player(Camera* cam);

    void spawn(glm::vec3 spawnPos, bool awake);

    float getHealth();
    float getMaxHealth();
    float getDamage();

    bool isDead();
    void addHealth(float amount);

    void update(GLfloat delta) override;
    void onCollision(Tag tag, Entity* other) override;


private:
    // calculate burn damage from stepping on lava
    glm::vec3 getMovementDir();
    
    void updateTimers(float delta);
    // checks inputs and returns target move direction
    glm::vec3 checkInputs(); 
    void shoot();

    void updateModel(glm::vec3 targetDir, float delta);
    void flyModeMovement(glm::vec3 targetDir);

    bool recentlyGrounded();
    void calculateBurnDamage(float delta);
    void checkClampXZVel();
    void checkJumpAndBoost();
    void calculateMovement(glm::vec3 targetDir, float delta);

    Camera* cam;
    glm::quat currRot;
    glm::quat targRot;
    glm::vec3 oldPos;

    float speed = 15.0f;            // movement speed of player
    float jumpSpeed = 25.0f;        // jump speed of player
    const float jumpLenience = 0.2f;  // still jump if player tries a little before allowed
    const float groundedLenience = 0.25f;  // potentially allows player to count as grounded a little after he actually is
    float boostSpeed = 60.0f;       // rocket boost speed
    float boostParticleTime = 0.0f; // how much longer to spawn boost particles
    float boostTimer = 0.0f;        // timer on boost recharge
    const float boostCooldown = 0.5f; // how long it takes for boost to recharge
    float maxHealth = 100.0f;       // maximum health player can have
    float health = maxHealth;       // current health of player 
    float attackDamage = 10.0f;     // how much damage player does per attack
    float shotsPerSecond = 1.0f;    // the rate player can shoot at
    float shootSpeed = 25.0f;       // how fast player shoots
    float invulnTime = 0.0f;        // how much remaining invulnerability player has    
    float timeSinceHitJump = 10.0f; // how long since player hit jump
    float timeSinceShot = 10.0f;    // how long since player shot
    float timeSinceGrounded = 10.0f;  // how long since player has been grounded
    float burnTime = 0.0f;          // how much longer player must burn

};