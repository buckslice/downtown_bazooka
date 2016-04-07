#include "projectile.h"
#include "graphics.h"
#include "entityManager.h"

Projectile::Projectile() {
    Collider* c = getCollider();
    c->enabled = false;
    c->type = ColliderType::TRIGGER;
    c->setExtents(glm::vec3(-0.5f), glm::vec3(0.5f));

    getTransform()->setVisibility(HIDDEN);
    getTransform()->color = glm::vec3(1.0f, 0.2f, 0.0f);

    Physics::setCollisionCallback(this);
}

Projectile::~Projectile() {
}

void Projectile::init(int id, glm::vec3 pos, glm::vec3 vel) {

    Transform* t = getTransform();
    Collider* c = getCollider();
    t->setPos(pos);
    t->setVisibility(VISIBLE);

    c->vel = vel;
    c->enabled = true;

    this->id = id;
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
        EntityManagerInstance->SpawnParticle(getTransform()->getWorldPos(), FIRE, 3.0f);
        break;
    case ProjectileType::LASER:
        for (int i = 0; i < 2; ++i) {
            EntityManagerInstance->SpawnParticle(getTransform()->getWorldPos(), BEAM, 10.0f, Mth::randInsideUnitCube());
        }
        break;
    default:
        break;
    }

}

void Projectile::onDeath() {
    EntityManagerInstance->ReturnProjectile(id);
    if (getCollider()->tag == Tag::PLAYER_PROJECTILE) {
        EntityManagerInstance->MakeExplosion(getTransform()->getWorldPos(), 100, 16.0f, getCollider()->vel);
        glm::vec3 p = getTransform()->getWorldPos();
        glm::vec3 s = glm::vec3(20.0f) / 2.0f;
        Physics::sendOverlapEvent(AABB(p - s, p + s), CollisionData{ ColliderType::BASIC, Tag::EXPLOSION });
    }
}

void Projectile::onCollision(CollisionData data) {
    Tag tag = getCollider()->tag;
    if (data.tag == Tag::PLAYER && tag != Tag::PLAYER_PROJECTILE) {
        onDeath();
    } else if (data.tag == Tag::ENEMY && tag != Tag::ENEMY_PROJECTILE) {
        onDeath();
    }
}
