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
        Physics::sendOverlapEvent(AABB(p - s, p + s), Tag::EXPLOSION , nullptr );
    }
    EntityManagerInstance->ReturnProjectile(this);
}

void Projectile::onCollision(Tag tag, Entity* other) {
    if ((tag == Tag::PLAYER && collider->tag != Tag::PLAYER_PROJECTILE) ||
        (tag == Tag::ENEMY && collider->tag != Tag::ENEMY_PROJECTILE)) {
        timer = -1.0f;
    }
}
