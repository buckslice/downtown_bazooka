#pragma once

#include "player.h"
#include "entity.h"
#include "mathutil.h"

class Enemy : public Entity {
public:
    Enemy(int player, glm::vec3 start, glm::vec3 scale) :Entity(start, scale) {
        this->player = player;
        speed = Mth::rand01() * 5.0f + 5.0f;
        jumpVel = Mth::rand01() * 10.0f + 20.0f;
    }

    void update(GLfloat delta) override;

    float speed;
    float jumpVel;

private:
    int player;
    float jumpTimer;
};