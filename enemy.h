#pragma once

#include "player.h"
#include "entity.h"
#include "mathutil.h"

const float SHOOT_VECINITY = 100.0f; // how close the enemies have to be to the player in order to shoot
const float AIM_DEVIANCE = 20.0f; // the factor of how much the enemies' aim deviates from the actual position of the player

enum class EnemyType {
    BASIC,
    ELITE,
    FLYER
};

class Enemy : public ModelEntity {
public:
    Enemy();
    ~Enemy();

    void update(GLfloat delta) override;

    void onCollision(Tag tag, Entity* other) override;

    void init(Transform* player, glm::vec3 pos, EnemyType type);

private:
    Transform* player = nullptr;

    float speed;
    float jumpVel;
    float health;
    float jumpTimer;
    float shootTimer;
};