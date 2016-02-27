#pragma once

#include "player.h"
#include "entity.h"
#include "mathutil.h"

class Enemy : public Entity {
public:
    Enemy(int player, glm::vec3 start, glm::vec3 scale);

    void update(GLfloat delta) override;

    float speed;
    float jumpVel;

private:
    int player;
    float jumpTimer;
};