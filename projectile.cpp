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
    timer = 2.0f;
}

void Projectile::update(GLfloat delta) {
    timer -= delta;
    if (timer <= 0.0f && id >= 0) {
        EntityManagerInstance->ReturnProjectile(id);
        id = -1;
        return;
    }
	if(timer <= .1f)
		EntityManagerInstance->MakeExplosion(getTransform()->getWorldPos());

    glm::vec3 pos = getTransform()->getWorldPos();

    for (int i = 0; i < 1; i++) {
        EntityManagerInstance->SpawnParticle(pos, Particle::FIRE, 3);
    }
}
