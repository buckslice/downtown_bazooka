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

    const int MAX_PROJECTILES = 100;
    const int MAX_PARTICLES = 5000;

    void init(int numberOfDudes);

    void update(float delta);

    void deleteEntities();


    int curParticle = 0;
    Particle *getNextParticle();

    void SpawnParticle(glm::vec3 pos, int effect = Particle::SPARK, float randvel = 0, glm::vec3 vel = glm::vec3(0, 0, 0));

    void SpawnProjectile(glm::vec3 pos, glm::vec3 vel);
    void ReturnProjectile(int id);


    ~EntityManager() {
        deleteEntities();
    }

private:

    std::vector<Entity*> entities;

    // particles can just be a vector since it doesnt really matter if one is replaced
    std::vector<Particle> particles;
    Pool<Projectile>* projectiles;

    Player* player;
};

//Global variable for the EntityManager
extern EntityManager *EntityManagerInstance;
