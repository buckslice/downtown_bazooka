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

    const int MAX_PROJECTILES = 1000;
    const int MAX_PARTICLES = 5000;

    // should add in support for adding single entities dynamically next
    // main problem is need to redo the graphics to just create buffer with fixed MAX_ENTITIES size
    // then update it each time a new entity is spawned using glBufferSubData()
    // or maybe glMapBuffer() or glMapBufferRange()
    void init(int numberOfDudes);

    void update(float delta);

    void deleteEntities();
    //void deleteEntities(int startIndex) {
    //    if (startIndex < entities.size()) {
    //        for (int i = startIndex; i < entities.size(); i++) {
    //            delete entities[i];
    //        }
    //        entities.erase(entities.begin() + startIndex, entities.end());
    //    }
    //}


    int curParticle = 0;
    Particle *getNextParticle();

    void SpawnParticle(glm::vec3 pos, int effect = Particle::SPARK, float randvel = 0, glm::vec3 vel = glm::vec3(0, 0, 0));

    ~EntityManager() {
        deleteEntities();
    }

    GLuint dudeMesh;
private:
    GLuint projectileMesh;

    std::vector<Entity*> entities;
    std::vector<Particle> particles;
    Player* player;
};

//Global variable for the EntityManager
extern EntityManager *EntityManagerInstance;
