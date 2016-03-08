#include "entityManager.h"

// TODO maybe make everything like this, singletons instead of static function calls?
// except this is kind of a shitty singleton
EntityManager *EntityManagerInstance;//An extern from entityManager.h

EntityManager::EntityManager(Player* player) : player(player) {

    // init particles
    particles.clear();
    for (int i = 0; i < MAX_PARTICLES; i++) {
        Particle p;
        particles.push_back(p);
    }

    // should switch this from pool ?
    projectiles = new Pool<Projectile>(MAX_PROJECTILES);
    enemies = new Pool<Enemy>(2000);

}

void EntityManager::init(int numberOfDudes) {
    EntityManagerInstance = this;

    for (int i = 0; i < numberOfDudes; i++) {
        SpawnEnemy();
    }

}

// should make like a PooledEntity child class of Entity or something
// this is pretty filth for now but whatever
void EntityManager::returnAllObjects() {
    for (size_t i = 0, len = projectiles->getSize(); i < len; ++i) {
        ReturnProjectile(i);
    }
    for (size_t i = 0, len = enemies->getSize(); i < len; ++i) {
        ReturnEnemy(i);
    }
}

void EntityManager::update(float delta) {
    player->update(delta);

    // update enemies
    auto& eobjs = enemies->getObjects();
    for (size_t i = 0, len = eobjs.size(); i < len; ++i) {
        if (eobjs[i].id < 0) {
            continue;
        }
        eobjs[i].data.update(delta);
    }

    // update projectiles
    auto& pobjs = projectiles->getObjects();
    for (size_t i = 0, len = pobjs.size(); i < len; ++i) {
        if (pobjs[i].id < 0) {
            continue;
        }
        pobjs[i].data.update(delta);
    }

    // update particles
    for (size_t i = 0, len = particles.size(); i < len; ++i) {
        particles[i].update(delta);
    }

}

// particles
Particle* EntityManager::getNextParticle() {
    Particle *p = &particles[curParticle];
    curParticle = (++curParticle) % (MAX_PARTICLES - 1);
    return p;
}

Particle* EntityManager::SpawnParticle(glm::vec3 pos, int effect, float mag, glm::vec3 vel) {
    Particle* p = getNextParticle();
    p->effect = effect;
    p->activate();
    p->getCollider()->vel = vel + Mth::randInsideSphere(1.0f) * mag;
    Transform* t = p->getTransform();
    t->setPos(pos);
    t->setScale(glm::vec3(.25f));
    return p;
}

void EntityManager::MakeExplosion(glm::vec3 pos, int num, float mag, glm::vec3 vel) {
    for (int i = 0; i < num; ++i) {
        SpawnParticle(pos, Particle::SPARK, mag * 5.0f, vel);
        SpawnParticle(pos, Particle::FIRE, mag*4, vel);
    }
}

// projectiles
void EntityManager::SpawnProjectile(glm::vec3 pos, glm::vec3 vel) {
    int id = projectiles->get();
    if (id < 0) {  // happens if pool is empty
        return;
    }
    projectiles->getData(id)->init(id, pos, vel);;
}

void EntityManager::SpawnEnemy() {
    int id = enemies->get();
    if (id < 0) {
        return;
    }

    Enemy* e = enemies->getData(id);

    glm::vec2 rnd = Mth::randomPointInSquare(CITY_SIZE);
    bool elite = rand() % 50 == 0;

    glm::vec3 scale = glm::vec3(1.0f, 2.0f, 1.0f);
    glm::vec3 variance = Mth::randInsideUnitCube();
    variance.x = variance.z = abs(variance.x);
    scale += variance * .25f;

    glm::vec3 color;
    if (elite) {
        scale *= Mth::rand01() + 2.0f;
        color = glm::vec3(0.8f, 1.0f, 0.6f);
        e->speed = Mth::rand01() * 5.0f + 10.0f;
        e->jumpVel = Mth::rand01() * 10.0f + 30.0f;
    } else {
        color = glm::vec3(1.0f, Mth::rand01() * 0.3f, Mth::rand01() * 0.3f);
        e->speed = Mth::rand01() * 5.0f + 5.0f;
        e->jumpVel = Mth::rand01() * 10.0f + 20.0f;
    }

    e->init(id, player->transform, glm::vec3(rnd.x, 200.0f, rnd.y), scale, color);

}

// should make a sub class of entity / template this or some shit later
void EntityManager::ReturnProjectile(int id) {
    ReturnPooledEntity(id, projectiles);
}

void EntityManager::ReturnEnemy(int id) {
    ReturnPooledEntity(id, enemies);
}

template <class Entity>
void EntityManager::ReturnPooledEntity(int id, Pool<Entity>* pool) {
    auto& objs = pool->getObjects();
    if (id < 0 || objs[id].id < 0) {
        return;
    }

    objs[id].data.getCollider()->awake = false;
    objs[id].data.getTransform()->setVisibility(HIDDEN);
    pool->ret(id);
}