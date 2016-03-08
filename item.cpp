#include "item.h"
#include "entityManager.h"
#include "graphics.h"


Item::Item() {
	model = Graphics::getTransform(Graphics::registerTransform(false));
	model->setVisibility(VISIBLE);

	getTransform()->setVisibility(HIDDEN_SELF);
	getTransform()->parentAll(model);
	model->setPos(0.0f, 0.5f, 0.0f);
	getCollider()->type = TRIGGER;
	getCollider()->tag = ITEM;
	getCollider()->awake = false;

	Physics::setCollisionCallback(this);
}


Item::~Item() {
}

void Item::update(GLfloat delta) {
	model->rotate(20.0f*delta, glm::vec3(1.0f, 0.0f, 1.0f));
}

void Item::onCollision(Collider * other) {
	if (other->tag == PLAYER) {
		EntityManagerInstance->ReturnItem(id);
	}
}

void Item::init(int id, float rotSpeed, glm::vec3 pos, glm::vec3 scale, glm::vec3 color) {
	Transform* t = getTransform();
	Collider* c = getCollider();
	t->setPos(pos);

	c->awake = true;
	c->tag = ITEM;

	this->id = id;
	this->rotSpeed = rotSpeed;
}
