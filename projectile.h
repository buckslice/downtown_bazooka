#pragma once
#include "entity.h"
enum ProjectileType {
	ROCKET,
	LASER
};

class Projectile : public Entity {
public:
    Projectile();
    ~Projectile();

    void update(GLfloat delta) override;

    void onCollision(CollisionData data) override;

	void onDeath();

    void init(int id, glm::vec3 pos, glm::vec3 vel);

	ProjectileType type;

private:
    int id;
    float timer;
};

