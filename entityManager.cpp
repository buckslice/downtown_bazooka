#include "entityManager.h"

// TODO maybe make everything like this, singletons instead of static function calls?
EntityManager *EntityManagerInstance;//An extern from entityManager.h

EntityManager::EntityManager(Player* player) : player(player) {

    // init particles
    particles.clear();
    for (int i = 0; i < MAX_PARTICLES; i++) {
        Particle p;
        particles.push_back(p);
    }

    projectiles = new Pool<Projectile>(MAX_PROJECTILES);
}

void EntityManager::init(int numberOfDudes) {
    EntityManagerInstance = this;

    // TODO ADD DUDES BACK IN (some code will prob go in their constructors now tho)
    //deleteEntities();

    //for (int i = 0; i < numberOfDudes; i++) {
    //    glm::vec2 rnd = Mth::randomPointInSquare(CITY_SIZE);
    //    bool elite = rand() % 50 == 0;

    //    glm::vec3 scale = glm::vec3(1.0f, 2.0f, 1.0f);
    //    glm::vec3 variance = Mth::randInsideUnitCube();
    //    variance.x = variance.z = abs(variance.x);
    //    scale += variance * .25f;
    //    if (elite) {
    //        scale *= Mth::rand01() + 2.0f;
    //    }
    //    Enemy* e = new Enemy(player->transform, glm::vec3(rnd.x, 200.0f, rnd.y), scale);

    //    if (elite) { // gold elites 
    //        e->speed = Mth::rand01() * 5.0f + 10.0f;
    //        e->jumpVel = Mth::rand01() * 10.0f + 30.0f;
    //        colors.push_back(glm::vec3(0.8f, 1.0f, 0.6f));
    //    } else {    // red grunts
    //        colors.push_back(glm::vec3(1.0f, Mth::rand01() * 0.3f, Mth::rand01() * 0.3f));
    //    }

}

void EntityManager::update(float delta) {
    player->update(delta);

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
        Particle& p = particles[i];
        p.update(delta);
    }

}


void EntityManager::deleteEntities() {
    for (size_t i = 0, len = entities.size(); i < len; ++i) {
        delete entities[i];
    }
    entities.erase(entities.begin(), entities.end());
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
        SpawnParticle(pos, Particle::FIRE, mag, vel);
    }
}



// projectiles

void EntityManager::SpawnProjectile(glm::vec3 pos, glm::vec3 vel) {
    int pin = projectiles->get();
    if (pin < 0) {  // happens if pool is empty
        return;
    }

    Projectile* p = projectiles->getData(pin);
    p->init(pin);
    p->getTransform()->setPos(pos);
    p->getCollider()->vel = vel;
}

void EntityManager::ReturnProjectile(int id) {
    Projectile* p = projectiles->getData(id);
    p->getCollider()->awake = false;
    p->getTransform()->visible = false;
    projectiles->ret(id);
}