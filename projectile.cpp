#include "projectile.h"
#include "graphics.h"
#include "entityManager.h"

Projectile::Projectile(){
	owner = nullptr;

    Collider* c = getCollider();
    c->awake = false;
    c->type = TRIGGER;
    c->setExtents(glm::vec3(-0.5f), glm::vec3(0.5f));

    getTransform()->setVisibility(HIDDEN);
    getTransform()->color = glm::vec3(1.0f, 0.2f, 0.0f);

    Physics::setCollisionCallback(this);
}

Projectile::~Projectile() {
}

void Projectile::init(int id, Entity *owner, glm::vec3 pos, glm::vec3 vel){
	setOwner(owner);

    Transform* t = getTransform();
    Collider* c = getCollider();
    t->setPos(pos);
    t->setVisibility(VISIBLE);

    c->vel = vel;
    c->awake = true;

    this->id = id;
    timer = 2.0f;
}

void Projectile::update(GLfloat delta) {
    timer -= delta;
    if (timer <= 0.0f) {
		onDeath();
        return;
    }
	switch (type)
	{
	case ROCKET:
		EntityManagerInstance->SpawnParticle(getTransform()->getWorldPos(), FIRE, 3.0f);
		break;
	case ProjectileType::LASER:
		for (int i = 0; i < 2; ++i) {
			EntityManagerInstance->SpawnParticle(getTransform()->getWorldPos(), BEAM, 10.0f, Mth::randInsideUnitCube());
		}
		break;
	default:
		break;
	}
	
}

void Projectile::onDeath(){
    EntityManagerInstance->ReturnProjectile(id);
	if (getCollider()->tag == PLAYER_PROJECTILE) {
		EntityManagerInstance->MakeExplosion(getTransform()->getWorldPos(), 100, 16.0f, getCollider()->vel);
	}
}

void Projectile::onCollision(Collider* other) {
	if(getOwner() != nullptr && other == getOwner()->getCollider())
		return;
	if (other->tag == PLAYER || other->tag == ENEMY) {
		onDeath();
	}
}
