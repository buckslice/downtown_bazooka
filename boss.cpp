#include "boss.h"
#include "physics.h"


Boss::Boss(glm::vec3 pos) {
	body = registerModel();
	body->setPos(pos);
	body->setScale(50.0f, 50.0f, 50.0f);
	brain = registerModel();
	transform->parentAll(body, brain);
	collider->type = ColliderType::FULL;

	Physics::setCollisionCallback(this);
}


Boss::~Boss() {
	
}

void Boss::update(GLfloat delta) {

}

void Boss::onCollision(Tag tag, Entity* other) {

}
