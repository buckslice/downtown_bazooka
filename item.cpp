#include "item.h"
#include "entityManager.h"
#include "graphics.h"


Item::Item() {
	model = Graphics::registerTransform();
	model->setVisibility(VISIBLE);
	model->setPos(0.0f, 1.0f, 0.0f);
	model->setRot(glm::vec3(45.0f, 0.0f, 45.0f));
	transform->setVisibility(HIDDEN);
	transform->parentAll(model);
	collider->type = ColliderType::TRIGGER;

	Physics::setCollisionCallback(this);
}


Item::~Item() {
    Graphics::returnTransform(model);
}

void Item::update(GLfloat delta) {
	timer += delta;
	transform->rotate(rotSpeed*delta, glm::vec3(0.0f, 1.0f, 0.0f));
	model->setPos(0.0f, 1.0f + sin(timer * 2.0f) * 0.5f, 0.0f);
}

void Item::onCollision(CollisionData data) {
	if (data.tag == Tag::PLAYER) {
		EntityManagerInstance->ReturnItem(this);
	}
}

void Item::init(float rotSpeed, glm::vec3 pos, ItemType type) {
	transform->setPos(pos);
	transform->setVisibility(HIDDEN_SELF);

	this->rotSpeed = rotSpeed;
	this->type = type;

    collider->tag = Tag::ITEM;
	switch (type) {
    case ItemType::HEAL:
		model->color = glm::vec3(0.0, 1.0, 0.0); //green
		break;
	case ItemType::STAMINA:
		model->color = glm::vec3(1.0, 0.0, 0.0); //red
		break;
	case ItemType::STRENGTH:
		model->color = glm::vec3(0.0, 1.0, 1.0); //cyan
		break;
	case ItemType::AGILITY:
		model->color = glm::vec3(1.0, 1.0, 0.0); //yellow
		break;
	case ItemType::DEXTERITY:
		model->color = glm::vec3(1.0, 1.0, 1.0); //white
		break;
	}
}
