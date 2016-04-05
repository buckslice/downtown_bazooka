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
	getCollider()->enabled = false;

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

void Item::init(int id, float rotSpeed, glm::vec3 pos, ItemType type) {
	Transform* t = getTransform();
	t->setPos(pos);
	t->setVisibility(HIDDEN_SELF);

	Collider* c = getCollider();
	c->enabled = true;

	this->id = id;
	this->rotSpeed = rotSpeed;
	this->type = type;

	switch (type) {
	case HEAL:
		model->color = glm::vec3(0.0, 1.0, 0.0); //green
		c->tag = ITEM_HEAL;
		break;
	case STAMINA:
		model->color = glm::vec3(1.0, 0.0, 0.0); //red
		c->tag = ITEM_STAMINA;
		break;
	case STRENGTH:
		model->color = glm::vec3(0.0, 1.0, 1.0); //cyan
		c->tag = ITEM_STRENGTH;
		break;
	case AGILITY:
		model->color = glm::vec3(1.0, 1.0, 0.0); //yellow
		c->tag = ITEM_AGILITY;
		break;
	case DEXTERITY:
		model->color = glm::vec3(1.0, 1.0, 1.0); //white
		c->tag = ITEM_DEXTERITY;
		break;
	}
}
