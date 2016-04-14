#include "entityManager.h"

// TODO maybe make everything like this, singletons instead of static function calls?
// except this is kind of a shitty singleton
EntityManager *EntityManagerInstance;//An extern from entityManager.h

EntityManager::EntityManager(Player* player) : player(player) {

    // init particles
    // careful with stuff like this because don't want to make a temp obj and call destructor with pooled stuff
    // i tried deleting the copy constructor in entity where this happens but then this function cant be
    // used at all. Probably need to rethink pool strategy a bit to be less error prone
    particles.resize(MAX_PARTICLES);

    // should switch this from pool ?
    projectiles = new MemPool<Projectile>(MAX_PROJECTILES);
    enemies = new MemPool<Enemy>(MAX_ENEMIES);
    items = new MemPool<Item>(MAX_ITEMS);

}

void EntityManager::init(int numberOfDudes) {
    EntityManagerInstance = this;

    for (int i = 0; i < numberOfDudes; i++) {
        SpawnEnemy();
    }
    for (int i = 0; i < numberOfDudes; i++) {
        SpawnItem();
    }
}

// should make like a PooledEntity child class of Entity or something
// this is pretty filth for now but whatever
void EntityManager::returnAllObjects() {
    projectiles->freeAll();
    enemies->freeAll();
    items->freeAll();
}

void EntityManager::update(float delta) {
    player->update(delta);

    // update enemies
    for (Enemy* e = nullptr; enemies->next(e);) {
        e->update(delta);
    }

    // update items
    for (Item* i = nullptr; items->next(i);) {
        i->update(delta);
    }

    // update projectiles
    for (Projectile* p = nullptr; projectiles->next(p);) {
        p->update(delta);
    }

    // update particles
    for (size_t i = 0, len = particles.size(); i < len; ++i) {
        particles[i].update(delta);
    }

}

float EntityManager::getPlayerDamage() {
    return player->getDamage();
}

// particles
Particle* EntityManager::getNextParticle() {
    Particle *p = &particles[curParticle];
    curParticle = (++curParticle) % (MAX_PARTICLES - 1);
    return p;
}

Particle* EntityManager::SpawnParticle(glm::vec3 pos, ParticleType effect, float mag, glm::vec3 vel) {
    Particle* p = getNextParticle();
    p->type = effect;
    p->activate();
    p->collider->vel = vel + Mth::randInsideSphere(1.0f) * mag;
    p->transform->setPos(pos);
    p->transform->setScale(glm::vec3(.25f));
    return p;
}


void EntityManager::MakeExplosion(glm::vec3 pos, int num, float mag, glm::vec3 vel) {
    AudioInstance->playSound(Resources::get().explosionSound);
    for (int i = 0; i < num; ++i) {
        SpawnParticle(pos, SPARK, mag * 5.0f, vel);
        SpawnParticle(pos, FIRE, mag * 4, vel);
    }
}

// projectiles
void EntityManager::SpawnProjectile(glm::vec3 pos, glm::vec3 vel, bool forPlayer) {
    Projectile* p = projectiles->alloc();
    if (!p) {
        return; // just ignore cuz pool is empty
    }
    p->collider->tag = forPlayer ? Tag::PLAYER_PROJECTILE : Tag::ENEMY_PROJECTILE;
    p->type = forPlayer ? ProjectileType::ROCKET : ProjectileType::LASER;
    p->init(pos, vel);
}

void EntityManager::SpawnEnemy() {
    Enemy* e = enemies->alloc();
    if (!e){
        return;
    }
    //glm::vec2 rnd = Mth::randomPointInSquare(CITY_SIZE);
    glm::vec2 rnd = Mth::randomPointInSquare(500.0f);
    EnemyType et = Mth::rand01() < 0.02f ? EnemyType::ELITE : EnemyType::BASIC;
    e->init(player->transform, glm::vec3(rnd.x, SPAWN_HEIGHT, rnd.y), et);
}


void EntityManager::SpawnItem() {
    Item* i = items->alloc();
    if (!i) {
        return;
    }
    glm::vec2 rnd = Mth::randomPointInSquare(CITY_SIZE);
    i->init(60.0f, glm::vec3(rnd.x, SPAWN_HEIGHT, rnd.y), (ItemType)(int)Mth::rand0X((int)ItemType::COUNT));
}

// should make a sub class of entity / template this or some shit later
void EntityManager::ReturnProjectile(Projectile* p) {
    projectiles->free(p);
}

void EntityManager::ReturnEnemy(Enemy* e) {
    enemies->free(e);
}

void EntityManager::ReturnItem(Item* i) {
    items->free(i);
}
