#pragma once
#include "entity.h"
enum class ProjectileType {
	ROCKET,
	LASER
};

class Projectile : public Entity {
public:
    Projectile();
    ~Projectile();

    void update(GLfloat delta) override;

    void onCollision(CollisionData data) override;

    void init(glm::vec3 pos, glm::vec3 vel);

	ProjectileType type;

private:
	void onDeath();

    float timer;
};

