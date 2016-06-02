#pragma once
#include "entity.h"
enum class ProjectileType {
	ROCKET,
	LASER,
    BOSS_CANNON,
    BOSS_HOMING,
};

class Projectile : public Entity {
public:
    Projectile();
    ~Projectile();

    void update(GLfloat delta) override;

    void onCollision(Tag tag, Entity* other) override;

    void activate(ProjectileType type, glm::vec3 pos, glm::vec3 vel);

private:
	void onDeath();

	ProjectileType type;
    float timer;
};

