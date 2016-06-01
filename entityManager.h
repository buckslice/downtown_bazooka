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
#include "waypoint.h"
#include "boss.h"

const int MAX_PROJECTILES = 200;
const int MAX_PARTICLES = 10000;
const int MAX_ENEMIES = 2000;
const int MAX_ITEMS = 2000;

class EntityManager {
public:
    EntityManager(Player* player);

    ~EntityManager();

    void update(float delta);

    void returnAllObjects();

    // spawn a particle at world position pos
    // effect dictates appearance, lifetime, and velocity over time
    // mag is magnitude of randomized velocity
    // vel is initial velocity
    // scale dictates starting size of particle
    // hasCollision determines if particle collides with anything
    void SpawnParticle(ParticleType type, glm::vec3 pos, glm::vec3 vel = glm::vec3(0.0f),
        float rmag = 0.0f, glm::vec3 scale = glm::vec3(1.0f), bool hasCollision = true);

    // make explosion out of particles at position pos
    // num dictates how many particles in explosion
    void MakeExplosion(glm::vec3 pos, int num = 100, float mag = 16.0f, glm::vec3 vel = glm::vec3(0.0f));

    void SpawnProjectile(ProjectileType type, Tag tag, glm::vec3 pos, glm::vec3 vel);
    void ReturnProjectile(Projectile* p);

    void SpawnEnemy(glm::vec3 pos, EnemyType type);
    void ReturnEnemy(Enemy* e);

    void SpawnItem(glm::vec3 pos, ItemType type);
    void ReturnItem(Item* i);

	void spawnBoss(glm::vec3 pos);
    void deleteBoss();
    Boss* getBoss();

    float getPlayerDamage() const;

private:
    int nextParticleIndex = 0;

    // particles can just be a vector since it doesnt really matter if one is replaced
    std::vector<Particle> particles;
    MemPool<Item>* items;
    MemPool<Projectile>* projectiles;
    MemPool<Enemy>* enemies;

    Player* player;

    Waypoint* waypoint = nullptr;
	Boss* boss = nullptr;

};

//Global variable for the EntityManager
extern EntityManager *EntityManagerInstance;
