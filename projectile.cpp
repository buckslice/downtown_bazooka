#include "projectile.h"



Projectile::Projectile(glm::vec3 position, float speed, glm::vec3 forward) {
	this->speed = speed;
	this->forward = forward;
	PhysicsTransform* pt = getTransform();
	pt->vel = speed * forward;
	pt->pos = position;
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
