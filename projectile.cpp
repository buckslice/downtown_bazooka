#include "projectile.h"
#include "graphics.h"
#include "entityManager.h"

Projectile::Projectile(){
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

void Projectile::init(int id, glm::vec3 pos, glm::vec3 vel){
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

	//EntityManagerInstance->SpawnParticle(getTransform()->getWorldPos(), Particle::FIRE, 3.0f);
}

void Projectile::onDeath(){
    EntityManagerInstance->ReturnProjectile(id);
    EntityManagerInstance->MakeExplosion(getTransform()->getWorldPos(),100,16.0f,getCollider()->vel);
}

void Projectile::onCollision(Collider* other) {
	Entity::onCollision(other);
	onDeath();
}
