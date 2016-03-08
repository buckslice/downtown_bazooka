#pragma once
#include "entity.h"
class Projectile : public Entity {
public:
    Projectile();
    ~Projectile();

	virtual void setOwner(Entity *owner){this->owner = owner;}
	Entity *getOwner(){return owner;}

    void update(GLfloat delta) override;

    void onCollision(Collider* other) override;

	void onDeath();

    void init(int id, Entity *owner, glm::vec3 pos, glm::vec3 vel);

private:
    int id;
    float timer;
	Entity *owner;
};

