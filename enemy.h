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

void Enemy::update(GLfloat delta) {
    PhysicsTransform& pt = *getTransform();

    jumpTimer -= delta;
    if (pt.grounded && jumpTimer < 0.0f) {
        pt.vel.y = jumpVel;
        pt.grounded = false;
        jumpTimer += Mth::rand01() * 10.0f + 2.0f;
    }

    if (pt.vel.y != 0.0f) {
        pt.grounded = false;
    }

    glm::vec3 dirToPlayer = Physics::getTransform(player)->pos - pt.pos;
    dirToPlayer.y = 0.0f;
    if (dirToPlayer != glm::vec3(0.0f)) {
        dirToPlayer = glm::normalize(dirToPlayer) * speed;
    }

    pt.vel.x = dirToPlayer.x;
    pt.vel.z = dirToPlayer.z;

    pt.vel.y += GRAVITY * delta;
}