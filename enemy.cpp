#include "enemy.h"
#include "graphics.h"
#include "entityManager.h"

Enemy::Enemy() {
    model = Graphics::getTransform(Graphics::registerTransform(false));
    model->setVisibility(VISIBLE);

    getTransform()->setVisibility(HIDDEN);
    getTransform()->parentAll(model);
    getCollider()->type = FULL;
	getCollider()->tag = ENEMY;
    getCollider()->awake = false;

    jumpTimer = Mth::rand01() * 5.0f + 5.0f;
    shootTimer = Mth::rand01() * 5.0f + 5.0f;
    Physics::setCollisionCallback(this);
}

void Enemy::init(int id, int player, glm::vec3 pos, glm::vec3 scale, glm::vec3 color) {
    this->id = id;
    this->player = player;

    getTransform()->setVisibility(HIDDEN_SELF);
    getTransform()->setPos(pos);
    model->setPos(0.0f, scale.y / 2.0f, 0.0f);
    model->setScale(scale);
    model->color = color;

    Collider* c = getCollider();
    glm::vec3 min = glm::vec3(-0.5f, 0.0f, -0.5f)*scale;
    glm::vec3 max = glm::vec3(0.5f, 1.0f, 0.5f)*scale;
    c->setExtents(min, max);
    c->type = FULL;
	c->tag = ENEMY;
    c->awake = true;
}

void Enemy::update(GLfloat delta) {
    Collider& col = *getCollider();

    jumpTimer -= delta;
	shootTimer -= delta;
    if (col.grounded && jumpTimer < 0.0f) {
        col.vel.y = jumpVel;
        col.grounded = false;
        jumpTimer += Mth::rand01() * 10.0f + 2.0f;
    }


    if (player < 0) {
        return;
    }

    glm::vec3 dirToPlayer = Graphics::getTransform(player)->getWorldPos() - getTransform()->getWorldPos();
	glm::vec3 shootDir = dirToPlayer + Mth::randInsideUnitCube() * AIM_DEVIANCE;
	float distsq = glm::dot(dirToPlayer, dirToPlayer);
	if (shootDir != glm::vec3(0.0f)) {
		shootDir = glm::normalize(shootDir);
	}
    dirToPlayer.y = 0.0f;
    if (dirToPlayer != glm::vec3(0.0f)) {
        dirToPlayer = glm::normalize(dirToPlayer) * speed;
    }
	if (shootTimer < 0.0f && distsq <= SHOOT_VECINITY * SHOOT_VECINITY) {
		EntityManagerInstance->SpawnProjectile(getTransform()->getWorldPos(), getCollider()->vel + shootDir*40.0f, false);
        shootTimer = Mth::rand01() * 10.0f + 5.0f;
	}

    col.vel.x = dirToPlayer.x;
    col.vel.z = dirToPlayer.z;

}

void Enemy::onCollision(CollisionData data) {
    if (data.tag == PLAYER_PROJECTILE || data.tag == EXPLOSION) {
        EntityManagerInstance->ReturnEnemy(id);
    }
}