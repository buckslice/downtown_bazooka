#include "enemy.h"

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

	glm::vec3 dirToPlayer = Physics::getTransform(player)->lpos - pt.lpos;
	dirToPlayer.y = 0.0f;
	if (dirToPlayer != glm::vec3(0.0f)) {
		dirToPlayer = glm::normalize(dirToPlayer) * speed;
	}

	pt.vel.x = dirToPlayer.x;
	pt.vel.z = dirToPlayer.z;

	//pt.vel.y += GRAVITY * delta;
}