#include "projectile.h"



Projectile::Projectile(glm::vec3 pos, glm::vec3 vel) {
    PhysicsTransform* pt = getTransform();
    pt->pos = pos;
    pt->vel = vel;
}


Projectile::~Projectile() {

}

void Projectile::update(GLfloat delta) {
    // if the projectile collides with something, delete it
    timer -= delta;
    if (timer <= 0.0f) {
        alive = false;
    }
}

bool Projectile::isAlive() const {
    return alive;
}
