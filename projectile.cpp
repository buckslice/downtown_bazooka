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
        //Physics::returnDynamic(transform);
        return;
    }

    // 2 and 3
    for (int i = 0; i < 2; i++) {
        PhysicsTransform* pt = getTransform();
        glm::vec3 word = pt->getWorldPos();
        EntityManagerInstance->SpawnParticle(word, Particle::FIRE, 3);
    }
    for (int i = 0; i < 3; i++) {
        EntityManagerInstance->SpawnParticle(getTransform()->getWorldPos(), Particle::SPARK, 50);
    }
}
