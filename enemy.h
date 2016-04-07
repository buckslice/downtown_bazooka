#pragma once

#include "player.h"
#include "entity.h"
#include "mathutil.h"
const float SHOOT_VECINITY = 100.0f; // how close the enemies have to be to the player in order to shoot
const float AIM_DEVIANCE = 20.0f; // the factor of how much the enemies' aim deviates from the actual position of the player

enum class EnemyType {
    BASIC,
    ELITE
};

class Enemy : public Entity {
public:
    Enemy();

    void update(GLfloat delta) override;

    void onCollision(CollisionData data) override;

    void init(int id, int player, glm::vec3 pos, EnemyType type);

private:
    int id;
    int player = -1;
    float speed;
    float jumpVel;
    float health;
    float jumpTimer;
    float shootTimer;
    Transform* model;
};