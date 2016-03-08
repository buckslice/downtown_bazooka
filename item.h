#pragma once
#include "entity.h"

class Item : public Entity {
public:
	Item();
	~Item();

	void update(GLfloat delta) override;

	void onCollision(CollisionData data) override;

	void init(int id, float rotSpeed, glm::vec3 pos, glm::vec3 scale, glm::vec3 color);
private:
	int id;
	float rotSpeed;
	float timer;

	Transform* model;
};

