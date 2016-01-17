#include "projectile.h"
#include "graphics.h"
#include "entityManager.h"

Projectile::Projectile(glm::vec3 pos, glm::vec3 vel) : Entity(pos, glm::vec3(1.0f), vel) {
}

Projectile::~Projectile() {

}

void Projectile::update(GLfloat delta) {
    // if the projectile collides with something, delete it
    timer -= delta;
    if (timer <= 0.0f) {
        getTransform()->alive = false;
    }

    for (int j = 0; j < 2; j++)
        EntityManagerInstance->SpawnParticle(getTransform()->getPos(), Particle::FIRE, 3);//,projectiles[j].getTransform()->vel);
    for (int j = 0; j < 3; j++)
        EntityManagerInstance->SpawnParticle(getTransform()->getPos(), Particle::SPARK, 50);//,projectiles[j].getTransform()->vel);
}
