#pragma once
#include "entity.h"
class Projectile : public Entity
{
public:
	Projectile(glm::vec3 position, float speed, glm::vec3 forward);
	~Projectile();

	void update(GLfloat delta) override;

	bool isAlive() const;
private:
	float speed;
	glm::vec3 forward;
	float timer = 1.0f;
	bool alive = true;
};

