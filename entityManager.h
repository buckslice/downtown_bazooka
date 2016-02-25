#pragma once
#include <vector>
#include "entity.h"
#include "enemy.h"
#include "mathutil.h"
#include "Particle.h"
#include "resources.h"
#include "cityGenerator.h"

class EntityManager {
public:

    EntityManager(Player* player);

    ~EntityManager() {
        deleteEntities();
    }

    const int MAX_PROJECTILES = 100;
    const int MAX_PARTICLES = 5000;

    void init(int numberOfDudes);

    void update(float delta);

    void deleteEntities();

    int curParticle = 0;
    Particle *getNextParticle();

    // spawn a particle at world position pos
    // effect dictates appearance, lifetime, and velocity over time
    // mag is magnitude of randomized velocity
    // vel is initial velocity
    Particle* SpawnParticle(glm::vec3 pos, int effect = Particle::SPARK, float mag = 0, glm::vec3 vel = glm::vec3(0, 0, 0));

    // make explosion out of particles at position pos
    // num dictates how many particles in explosion
    void MakeExplosion(glm::vec3 pos, int num = 100, float mag = 16.0f, glm::vec3 vel = glm::vec3(0, 0, 0));

    void SpawnProjectile(glm::vec3 pos, glm::vec3 vel);
    void ReturnProjectile(int id);

private:

    std::vector<Entity*> entities;

    // particles can just be a vector since it doesnt really matter if one is replaced
    std::vector<Particle> particles;
    Pool<Projectile>* projectiles;

    Player* player;
};

//Global variable for the EntityManager
extern EntityManager *EntityManagerInstance;
