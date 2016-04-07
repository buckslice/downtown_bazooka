#include "enemy.h"
#include "graphics.h"
#include "entityManager.h"

Enemy::Enemy() {
    model = Graphics::getTransform(Graphics::registerTransform(false));
    model->setVisibility(VISIBLE);

    getTransform()->setVisibility(HIDDEN);
    getTransform()->parentAll(model);
    getCollider()->type = ColliderType::FULL;
    getCollider()->tag = Tag::ENEMY;
    getCollider()->enabled = false;

    jumpTimer = Mth::rand01() * 5.0f + 5.0f;
    shootTimer = Mth::rand01() * 5.0f + 5.0f;
    Physics::setCollisionCallback(this);
}

void Enemy::init(int id, int player, glm::vec3 pos, EnemyType type) {
    this->id = id;
    this->player = player;

    glm::vec3 scale = glm::vec3(1.0f, 2.0f, 1.0f);
    glm::vec3 variance = Mth::randInsideUnitCube();
    variance.x = variance.z = abs(variance.x);
    scale += variance * .25f;

    glm::vec3 color;
    switch (type) {
    case EnemyType::ELITE:
        scale *= Mth::rand01() + 2.0f;
        color = glm::vec3(0.8f, 1.0f, 0.6f);
        speed = Mth::rand01() * 5.0f + 10.0f;
        jumpVel = Mth::rand01() * 10.0f + 30.0f;
        health = 100.0f;
        break;
    case EnemyType::BASIC:
        color = glm::vec3(1.0f, Mth::rand01() * 0.3f, Mth::rand01() * 0.3f);
        speed = Mth::rand01() * 5.0f + 5.0f;
        jumpVel = Mth::rand01() * 10.0f + 20.0f;
        health = 20.0f;
        break;
    default:
        break;
    }

    getTransform()->setVisibility(HIDDEN_SELF);
    getTransform()->setPos(pos);
    model->setPos(0.0f, scale.y / 2.0f, 0.0f);
    model->setScale(scale);
    model->color = color;

    Collider* c = getCollider();
    glm::vec3 min = glm::vec3(-0.5f, 0.0f, -0.5f)*scale;
    glm::vec3 max = glm::vec3(0.5f, 1.0f, 0.5f)*scale;
    c->setExtents(min, max);
    c->type = ColliderType::FULL;
    c->tag = Tag::ENEMY;
    c->enabled = true;
}

void Enemy::update(GLfloat delta) {
    if (health <= 0.0f) {
        EntityManagerInstance->ReturnEnemy(id);
    }
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
    if (shootTimer < 0.0f) {
        shootTimer = Mth::rand01() * 10.0f + 5.0f;
        if (distsq <= SHOOT_VECINITY * SHOOT_VECINITY) {
            EntityManagerInstance->SpawnProjectile(getTransform()->getWorldPos(), getCollider()->vel + shootDir*40.0f, false);
        }
    }

    col.vel.x = dirToPlayer.x;
    col.vel.z = dirToPlayer.z;

}

void Enemy::onCollision(CollisionData data) {
    if (data.tag == Tag::PLAYER_PROJECTILE || data.tag == Tag::EXPLOSION) {
        health -= EntityManagerInstance->getPlayerDamage();
        //health -= dynamic_cast<Player*>(data.entity)->getDamage();
    }
}