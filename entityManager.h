#pragma once

// would make debug run way better but we need to recompile SFML with this option i think
// though it definitely seems like glm/glm.hpp is culprit but thats not even in a lib soo..
//#define _HAS_ITERATOR_DEBUGGING 0

#include <vector>
#include "entity.h"
#include "enemy.h"
#include "item.h"
#include "mathutil.h"
#include "particle.h"
#include "pool.h"

const int MAX_PROJECTILES = 200;
const int MAX_PARTICLES = 5000;
const int MAX_ENEMIES = 2000;
const int MAX_ITEMS = 2000;

class EntityManager {
public:
    EntityManager(Player* player);

    ~EntityManager() {
    }

    void init(int numberOfDudes);

    void update(float delta);

    void returnAllObjects();

    int curParticle = 0;
    Particle *getNextParticle();

    // spawn a particle at world position pos
    // effect dictates appearance, lifetime, and velocity over time
    // mag is magnitude of randomized velocity
    // vel is initial velocity
    Particle* SpawnParticle(glm::vec3 pos, ParticleType effect = SPARK, float mag = 0, glm::vec3 vel = glm::vec3(0, 0, 0));

    // make explosion out of particles at position pos
    // num dictates how many particles in explosion
    void MakeExplosion(glm::vec3 pos, int num = 100, float mag = 16.0f, glm::vec3 vel = glm::vec3(0, 0, 0));

    void SpawnProjectile(glm::vec3 pos, glm::vec3 vel, bool forPlayer);
    void ReturnProjectile(Projectile* p);

    void SpawnEnemy(glm::vec3 pos, EnemyType type);
    void ReturnEnemy(Enemy* e);

	void SpawnItem(glm::vec3 pos, ItemType type);
	void ReturnItem(Item* i);

    float getPlayerDamage();

private:
    // particles can just be a vector since it doesnt really matter if one is replaced
    std::vector<Particle> particles;
	MemPool<Item>* items;
    MemPool<Projectile>* projectiles;
    MemPool<Enemy>* enemies;

    Player* player;

};

//Global variable for the EntityManager
extern EntityManager *EntityManagerInstance;
// try implementing event system that entity manager can listen to!! at least for spawning of things
