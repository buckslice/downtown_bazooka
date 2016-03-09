#include "item.h"
#include "entityManager.h"
#include "graphics.h"


Item::Item() {
	model = Graphics::getTransform(Graphics::registerTransform(false));
	model->setVisibility(VISIBLE);
	model->setPos(0.0f, 1.0f, 0.0f);
	model->setRot(glm::vec3(45.0f, 0.0f, 45.0f));
	getTransform()->setVisibility(HIDDEN);
	getTransform()->parentAll(model);
	getCollider()->type = TRIGGER;
	getCollider()->tag = ITEM;
	getCollider()->awake = false;

	Physics::setCollisionCallback(this);
}


Item::~Item() {
}

void Item::update(GLfloat delta) {
	timer += delta;
	getTransform()->rotate(rotSpeed*delta, glm::vec3(0.0f, 1.0f, 0.0f));
	model->setPos(0.0f, 1.0f + sin(timer * 2.0f) * 0.5f, 0.0f);
}

void Item::onCollision(CollisionData data) {
	if (data.tag == PLAYER) {
		EntityManagerInstance->ReturnItem(id);
	}
}

void Item::init(int id, float rotSpeed, glm::vec3 pos, glm::vec3 scale, glm::vec3 color) {
	Transform* t = getTransform();
	t->setPos(pos);
	t->setVisibility(HIDDEN_SELF);

	Collider* c = getCollider();
	c->awake = true;
	c->tag = ITEM;

	this->id = id;
	this->rotSpeed = rotSpeed;
}