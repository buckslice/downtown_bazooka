#include "entityManager.h"
#include "audio.h"

// TODO maybe make everything like this, singletons instead of static function calls?
// except this is kind of a shitty singleton
EntityManager *EntityManagerInstance;//An extern from entityManager.h

EntityManager::EntityManager(Player* player) : player(player) {
    EntityManagerInstance = this;

    // init particles
    // careful with stuff like this because don't want to make a temp obj and call destructor with pooled stuff
    // i tried deleting the copy constructor in entity where this happens but then this function cant be
    // used at all. Should add more guards and checks in pool class to make it less error prone
    particles.resize(MAX_PARTICLES);

    // should switch this from pool ?
    projectiles = new MemPool<Projectile>(MAX_PROJECTILES);
    enemies = new MemPool<Enemy>(MAX_ENEMIES);
    items = new MemPool<Item>(MAX_ITEMS);

}

void EntityManager::init(int numberOfDudes) {
    //EntityManagerInstance = this;

    //for (int i = 0; i < numberOfDudes; i++) {
    //    glm::vec2 rnd = Mth::randomPointInSquare(500.0f);
    //    EnemyType et = Mth::rand01() < 0.02f ? EnemyType::ELITE : EnemyType::BASIC;
    //    SpawnEnemy(glm::vec3(rnd.x, 200.0f, rnd.y), et);
    //}
    //for (int i = 0; i < numberOfDudes; i++) {
    //    glm::vec2 rnd = Mth::randomPointInSquare(CITY_SIZE);
    //    SpawnItem(glm::vec3(rnd.x, 200.0f, rnd.y), (ItemType)Mth::randRange(0,(int)ItemType::COUNT));
    //}
}

void EntityManager::returnAllObjects() {
    projectiles->freeAll();
    enemies->freeAll();
    items->freeAll();
}

void EntityManager::update(float delta) {
    player->update(delta);

    waypoint.update(delta);

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

void EntityManager::SpawnParticle(ParticleType type, 
    glm::vec3 pos, glm::vec3 vel, float rmag, glm::vec3 scale, bool hasCollision) {

    particles[nextParticleIndex].activate(type, pos, vel, rmag, scale, hasCollision);;
    nextParticleIndex = ++nextParticleIndex % MAX_PARTICLES;
}


void EntityManager::MakeExplosion(glm::vec3 pos, int num, float mag, glm::vec3 vel) {
    AudioInstance->playSound(Resources::get().explosionSound);
    for (int i = 0; i < num; ++i) {
        SpawnParticle(SPARK, pos, vel, mag * 5.0f);
        SpawnParticle(FIRE, pos, vel, mag * 4.0f);
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

void EntityManager::SpawnEnemy(glm::vec3 pos, EnemyType type) {
    Enemy* e = enemies->alloc();
    if (!e) {
        return;
    }
    e->init(player->transform, pos, type);
}


void EntityManager::SpawnItem(glm::vec3 pos, ItemType type) {
    Item* i = items->alloc();
    if (!i) {
        return;
    }
    i->init(pos, type);
}

void EntityManager::ReturnProjectile(Projectile* p) {
    projectiles->free(p);
}

void EntityManager::ReturnEnemy(Enemy* e) {
    enemies->free(e);
}

void EntityManager::ReturnItem(Item* i) {
    items->free(i);
}
