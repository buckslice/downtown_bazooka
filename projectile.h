#pragma once
#include "entity.h"
class Projectile : public Entity
{
public:
	Projectile(glm::vec3 pos, glm::vec3 vel);
	~Projectile();

	void update(GLfloat delta) override;

private:
	float timer = 5.0f;
};

