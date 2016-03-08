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
	glm::vec3 shootDir = dirToPlayer + Mth::randInsideUnitCube() * 5.0f;
	float distsq = glm::dot(dirToPlayer, dirToPlayer);
	float closeEnough = 100.0f;
	if (shootDir != glm::vec3(0.0f)) {
		shootDir = glm::normalize(shootDir);
	}
    dirToPlayer.y = 0.0f;
    if (dirToPlayer != glm::vec3(0.0f)) {
        dirToPlayer = glm::normalize(dirToPlayer) * speed;
    }
	// this is to make the enemies shoot, but it is incomplete because I wasn't sure where to tag the projectiles as ENEMY_PROJECTILE
	if (shootTimer < 0.0f && distsq <= closeEnough * closeEnough) {
		EntityManagerInstance->SpawnProjectile(this, getTransform()->getWorldPos(), getCollider()->vel + shootDir*40.0f, false);
		shootTimer = Mth::rand01() * 10.0f + 2.0f;
	}

    col.vel.x = dirToPlayer.x;
    col.vel.z = dirToPlayer.z;

}

void Enemy::onCollision(Collider* other) {
    if (other->tag == PLAYER_PROJECTILE) {
        EntityManagerInstance->ReturnEnemy(id);
    }
}