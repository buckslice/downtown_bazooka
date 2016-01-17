#include "Particle.h"

void Particle::activate() {
	getTransform()->alive = true;

	float gravmult;
	switch (effect) {
	case SPARK:
		gravmult = 5.0f;
		maxlifetime = 3.0f;
		break;
	case CLOUD:
		gravmult = -.25f;
		maxlifetime = .5f;
		break;
	case FIRE:
		gravmult = -.5f;
		maxlifetime = 2.0f;
		break;
	}
	this->getTransform()->gravityMultiplier = gravmult;
	lifetime = maxlifetime;
}

void Particle::update(GLfloat dt) {
	PhysicsTransform* pt = getTransform();

	if ((lifetime -= dt) <= 0) {
		pt->alive = false;
		return;
	}

	float scalemult;
	switch (effect) {
	case SPARK:
		scalemult = .97f;
		break;
	case CLOUD:
		if (lifetime > maxlifetime*.25f)
			scalemult = 1.02f;
		else
			scalemult = .9f;
		break;
	case FIRE:
		scalemult = .95f;
		break;
	}
	//pt->scale *= scalemult;
	pt->scale = (lifetime / maxlifetime) * glm::vec3(1.0f);
}

glm::vec3 Particle::getColor() {
	switch (effect) {
	case FIRE:
		return glm::vec3(1.0f, lifetime / maxlifetime, 0.0f);
	}
	return glm::vec3(1.0f);
}
