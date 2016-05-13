#include "enemy.h"
#include "graphics.h"
#include "entityManager.h"

Enemy::Enemy() {
    jumpTimer = Mth::rand01() * 5.0f + 5.0f;
    shootTimer = Mth::rand01() * 5.0f + 5.0f;

    transform->setVisibility(Visibility::HIDE_SELF);
    collider->type = ColliderType::FULL;
    collider->tag = Tag::ENEMY;

    Physics::setCollisionCallback(this);
}

Enemy::~Enemy() {
}

void Enemy::init(Transform* player, glm::vec3 pos, EnemyType type) {
    this->player = player;

    Transform* model = registerModel();
    transform->parentAll(model);

    glm::vec3 scale = glm::vec3(1.0f, 2.0f, 1.0f);
    glm::vec3 variance = Mth::randInsideUnitCube();
    variance.x = variance.z = abs(variance.x);
    scale += variance * .25f;

    glm::vec3 color;
    switch (type) {
    case EnemyType::ELITE: {
        scale *= Mth::rand01() + 2.0f;
        color = glm::vec3(0.8f, 1.0f, 0.6f);
        speed = Mth::rand01() * 5.0f + 10.0f;
        jumpVel = Mth::rand01() * 10.0f + 30.0f;
        health = 100.0f;

        Transform* larm = registerModel();
        Transform* rarm = registerModel();

        larm->setPos(-scale.x * 0.75f, scale.y * 0.75f, 0.0f);
        rarm->setPos(scale.x * 0.75f, scale.y * 0.75f, 0.0f);

        larm->setScale(scale*0.75f);
        rarm->setScale(scale*0.75f);

        transform->parentAllWithColor(larm, rarm);

        break; }
    case EnemyType::BASIC: {
        color = glm::vec3(1.0f, Mth::rand01() * 0.3f, Mth::rand01() * 0.3f);
        speed = Mth::rand01() * 5.0f + 5.0f;
        jumpVel = Mth::rand01() * 10.0f + 20.0f;
        health = 20.0f;

        //scale = glm::vec3(0.75f, 2.0f, 0.75f);
        Transform* head = registerModel();
        head->setPos(0.0f, scale.y * 0.75f, 0.0f);
        head->setScale(1.3f, 0.7f, 1.3f);
        head->color = glm::vec3(1.0f, 0.0f, 0.0f);
        transform->parentAll(head);


        break; }
    default:
        break;
    }

    transform->setPos(pos);
    model->setPos(0.0f, scale.y / 2.0f, 0.0f);
    model->setScale(scale);
    model->color = color;

    glm::vec3 min = glm::vec3(-0.5f, 0.0f, -0.5f)*scale;
    glm::vec3 max = glm::vec3(0.5f, 1.0f, 0.5f)*scale;
    collider->setExtents(min, max);
}

void Enemy::update(GLfloat delta) {
    if (health <= 0.0f || !collider->awake) {
        EntityManagerInstance->ReturnEnemy(this);
        return;
    }

    jumpTimer -= delta;
    shootTimer -= delta;
    if (collider->grounded && jumpTimer < 0.0f) {
        collider->vel.y = jumpVel;
        collider->grounded = false;
        jumpTimer += Mth::rand01() * 10.0f + 2.0f;
    }

    if (!player) {
        return;
    }

    glm::vec3 dirToPlayer = player->getWorldPos() - transform->getWorldPos();
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
            EntityManagerInstance->SpawnProjectile(transform->getWorldPos(), collider->vel + shootDir*40.0f, false);
        }
    }

    collider->vel.x = dirToPlayer.x;
    collider->vel.z = dirToPlayer.z;

}

void Enemy::onCollision(Tag tag, Entity* other) {
    if (tag == Tag::PLAYER_PROJECTILE || tag == Tag::EXPLOSION) {
        health -= EntityManagerInstance->getPlayerDamage();
    }
}