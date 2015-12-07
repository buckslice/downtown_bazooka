#include "projectile.h"
#include "graphics.h"

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

    //GetEntityManager()->SpawnParticle(getTransform()->getPos());
}
