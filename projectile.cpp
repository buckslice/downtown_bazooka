#include "projectile.h"
#include "graphics.h"
#include "entityManager.h"

Projectile::Projectile() :Entity() {
    getCollider()->awake = false;
    getTransform()->visible = false;
    getTransform()->color = glm::vec3(1.0f, 0.2f, 0.0f);
}

Projectile::~Projectile() {
}

void Projectile::init(int id) {
    getCollider()->awake = true;
    getTransform()->visible = true;
    this->id = id;
    timer = 5.0f;
}

void Projectile::update(GLfloat delta) {
    timer -= delta;
    if (timer <= 0.0f && id >= 0) {
        EntityManagerInstance->ReturnProjectile(id);
        id = -1;
        return;
    }

    glm::vec3 pos = getTransform()->getWorldPos();

    for (int i = 0; i < 2; i++) {
        EntityManagerInstance->SpawnParticle(pos, Particle::FIRE, 3);
    }
    for (int i = 0; i < 3; i++) {
        EntityManagerInstance->SpawnParticle(pos, Particle::SPARK, 50);
    }
}
