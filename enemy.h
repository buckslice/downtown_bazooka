#pragma once

#include "player.h"
#include "entity.h"
#include "mathutil.h"
const float SHOOT_VECINITY = 100.0f; // how close the enemies have to be to the player in order to shoot
const float AIM_DEVIANCE = 20.0f; // the factor of how much the enemies' aim deviates from the actual position of the player
const float MAX_ENEMY_HEALTH = 100.0f;
class Enemy : public Entity {
public:
    Enemy();

    void update(GLfloat delta) override;

    void onCollision(CollisionData data) override;

    void init(int id, int player, glm::vec3 pos, glm::vec3 scale, glm::vec3 color);

    float speed, jumpVel, health;

private:
    int id, player = -1;
    float jumpTimer, shootTimer;
    Transform* model;
};