#pragma once
#include <vector>
#include "entity.h"
#include "enemy.h"
#include "item.h"
#include "mathutil.h"
#include "Particle.h"
#include "resources.h"
#include "cityGenerator.h"

class EntityManager {
public:

    EntityManager(Player* player);

    ~EntityManager() {
    }

    const int MAX_PROJECTILES = 200;
    const int MAX_PARTICLES = 5000;
	const int MAX_ENEMIES = 1100;
    const int MAX_ITEMS = 1100;

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
    void ReturnProjectile(int id);

    void SpawnEnemy();
    void ReturnEnemy(int id);

	void SpawnItem();
	void ReturnItem(int id);

private:
    // particles can just be a vector since it doesnt really matter if one is replaced
    std::vector<Particle> particles;
	Pool<Item>* items;
    Pool<Projectile>* projectiles;
    Pool<Enemy>* enemies;

    Player* player;

    template <class Entity>
    void ReturnPooledEntity(int id, Pool<Entity>* pool);
};

//Global variable for the EntityManager
extern EntityManager *EntityManagerInstance;
// try implementing event system that entity manager can listen to!! at least for spawning of things
