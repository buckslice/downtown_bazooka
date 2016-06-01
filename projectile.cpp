#include "projectile.h"
#include "graphics.h"
#include "entityManager.h"

Projectile::Projectile() {
    collider->type = ColliderType::TRIGGER;
    collider->setExtents(glm::vec3(-0.5f), glm::vec3(0.5f));

    transform->setVisibility(Visibility::HIDE_ALL);
    transform->color = glm::vec3(1.0f, 0.2f, 0.0f);
    transform->shape = Shape::CUBE_SOLID;

    Physics::setCollisionCallback(this);
}

Projectile::~Projectile() {
}

void Projectile::activate(ProjectileType type, glm::vec3 pos, glm::vec3 vel) {
    this->type = type;
    transform->setPos(pos);
    transform->setVisibility(Visibility::SHOW_SELF);

    collider->vel = vel;

    timer = 2.0f;

    switch (type) {
    case ProjectileType::BOSS_CANNON:
        transform->setScale(glm::vec3(4.0f));
        collider->setExtents(glm::vec3(-2.0f), glm::vec3(2.0f));
        timer = 4.0f;
    case ProjectileType::ROCKET:
        collider->gravityMultiplier = 0.0f;
        break;
    case ProjectileType::LASER:
        collider->gravityMultiplier = 1.0f;
        break;
    default:
        break;
    }
}

void Projectile::update(GLfloat delta) {
    timer -= delta;
    if (timer <= 0.0f) {
        onDeath();
        return;
    }
    switch (type) {
    case ProjectileType::ROCKET:
        EntityManagerInstance->SpawnParticle(FIRE, transform->getWorldPos(), glm::vec3(0.0f), 3.0f);
        break;
    case ProjectileType::LASER:
        for (int i = 0; i < 2; ++i) {
            EntityManagerInstance->SpawnParticle(BEAM, transform->getWorldPos(), Mth::randInsideUnitCube(), 10.0f);
        }
        break;
    case ProjectileType::BOSS_CANNON:
        for (int i = 0; i < 1; ++i) {
            EntityManagerInstance->SpawnParticle(BEAM, transform->getWorldPos(), glm::vec3(0.0f), 10.0f, glm::vec3(8.0f), false);
        }
        break;
    default:
        break;
    }

}

void Projectile::onDeath() {
    if (collider->tag == Tag::PLAYER_PROJECTILE) {
        EntityManagerInstance->MakeExplosion(transform->getWorldPos(), 100, 16.0f, collider->vel);
        glm::vec3 p = transform->getWorldPos();
        glm::vec3 s = glm::vec3(20.0f) / 2.0f;
        Physics::sendOverlapEvent(AABB(p - s, p + s), Tag::EXPLOSION, nullptr);
    }
    if (type == ProjectileType::BOSS_CANNON) {
        for (int i = 0; i < 50; ++i) {
            EntityManagerInstance->SpawnParticle(BEAM, transform->getWorldPos(), glm::vec3(0.0f), 100.0f, glm::vec3(8.0f), false);
        }
    }
    EntityManagerInstance->ReturnProjectile(this);
}

void Projectile::onCollision(Tag tag, Entity* other) {
    if ((tag == Tag::PLAYER && collider->tag != Tag::PLAYER_PROJECTILE) ||
        (tag == Tag::ENEMY && collider->tag != Tag::ENEMY_PROJECTILE) ||
        tag == Tag::BOSSBOY) {
        timer = -1.0f;
    }
}
