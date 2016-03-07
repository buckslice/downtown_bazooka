#pragma once

#include "player.h"
#include "entity.h"
#include "mathutil.h"

class Enemy : public Entity {
public:
    Enemy();

    void update(GLfloat delta) override;

    void onCollision(Collider* other) override;

    void init(int id, int player, glm::vec3 pos, glm::vec3 scale, glm::vec3 color);

    float speed;
    float jumpVel;

private:
    int id;
    int player = -1;
    float jumpTimer;
	//float shootTimer;

    Transform* model;

};