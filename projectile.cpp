#include "projectile.h"
#include "graphics.h"
#include "entityManager.h"

Projectile::Projectile() {
    collider->type = ColliderType::TRIGGER;
    collider->setExtents(glm::vec3(-0.5f), glm::vec3(0.5f));

    transform->setVisibility(HIDDEN);
    transform->color = glm::vec3(1.0f, 0.2f, 0.0f);

    Physics::setCollisionCallback(this);
}

Projectile::~Projectile() {
}

void Projectile::init(glm::vec3 pos, glm::vec3 vel) {
    transform->setPos(pos);
    transform->setVisibility(VISIBLE);

    collider->vel = vel;

    timer = 2.0f;
}

void Projectile::update(GLfloat delta) {
    timer -= delta;
    if (timer <= 0.0f) {
        onDeath();
        return;
    }
    switch (type) {
    case ProjectileType::ROCKET:
        EntityManagerInstance->SpawnParticle(transform->getWorldPos(), FIRE, 3.0f);
        break;
    case ProjectileType::LASER:
        for (int i = 0; i < 2; ++i) {
            EntityManagerInstance->SpawnParticle(transform->getWorldPos(), BEAM, 10.0f, Mth::randInsideUnitCube());
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
        Physics::sendOverlapEvent(AABB(p - s, p + s), CollisionData{ ColliderType::BASIC, Tag::EXPLOSION });
    }
    EntityManagerInstance->ReturnProjectile(this);
}

void Projectile::onCollision(CollisionData data) {
    Tag tag = collider->tag;
    if (data.tag == Tag::PLAYER && tag != Tag::PLAYER_PROJECTILE) {
        onDeath();
    } else if (data.tag == Tag::ENEMY && tag != Tag::ENEMY_PROJECTILE) {
        onDeath();
    }
}
