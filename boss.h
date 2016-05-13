#pragma once
#include "entity.h"
class Boss : public ModelEntity {
public:
	Boss(glm::vec3 pos);
	~Boss();

	void update(GLfloat delta) override;

	void onCollision(Tag tag, Entity* other) override;
private:
	Transform* body;
	Transform* brain;
	int phase;
	int health;
};

