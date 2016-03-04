#include "enemy.h"
#include "graphics.h"
#include "entityManager.h"

Enemy::Enemy() {
    model = Graphics::getTransform(Graphics::registerTransform(false));
    model->setVisibility(VISIBLE);

    getTransform()->setVisibility(HIDDEN);
    getTransform()->parentAll(model);
    getCollider()->type = FULL;
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
    c->awake = true;
}

void Enemy::update(GLfloat delta) {
    Collider& col = *getCollider();

    jumpTimer -= delta;
    if (col.grounded && jumpTimer < 0.0f) {
        col.vel.y = jumpVel;
        col.grounded = false;
        jumpTimer += Mth::rand01() * 10.0f + 2.0f;
    }

    if (player < 0) {
        return;
    }

    glm::vec3 dirToPlayer = Graphics::getTransform(player)->getWorldPos() - getTransform()->getWorldPos();
    dirToPlayer.y = 0.0f;
    if (dirToPlayer != glm::vec3(0.0f)) {
        dirToPlayer = glm::normalize(dirToPlayer) * speed;
    }

    col.vel.x = dirToPlayer.x;
    col.vel.z = dirToPlayer.z;

}

void Enemy::onCollision(Collider* other) {
    if (other->type == TRIGGER) {
        EntityManagerInstance->ReturnEnemy(id);
    }
}