#pragma once
#include <vector>
#include "entity.h"
#include "enemy.h"
#include "mathutil.h"
#include "Particle.h"
#include "resources.h"

class EntityManager {
public:

    EntityManager(Player* player) : player(player) {
        dudeMesh = Graphics::registerMesh();
        projectileMesh = Graphics::registerMesh(Resources::get().solidTex);
        particleMesh = Graphics::registerMesh(Resources::get().solidTex);
    }

    const int MAX_PROJECTILES = 1000;
    const int MAX_PARTICLES = 5000;

    // should add in support for adding single entities dynamically next
    // main problem is need to redo the graphics to just create buffer with fixed MAX_ENTITIES size
    // then update it each time a new entity is spawned using glBufferSubData()
    // or maybe glMapBuffer() or glMapBufferRange()
    void init(int numberOfDudes) {
        deleteEntities();

        std::vector<glm::vec3> colors;
        for (int i = 0; i < numberOfDudes; i++) {
            glm::vec2 rnd = Mth::randomPointInSquare(CITY_SIZE);
            bool elite = rand() % 50 == 0;

            glm::vec3 scale = glm::vec3(1.0f, 2.0f, 1.0f);
            glm::vec3 variance = Mth::randInsideUnitSphere();
            variance.x = variance.z = abs(variance.x);
            scale += variance * .25f;
            if (elite) {
                scale *= Mth::rand01() + 2.0f;
            }
            Enemy* e = new Enemy(player->transform, glm::vec3(rnd.x, 200.0f, rnd.y), scale);

            if (elite) { // gold elites 
                e->speed = Mth::rand01() * 5.0f + 10.0f;
                e->jumpVel = Mth::rand01() * 10.0f + 30.0f;
                colors.push_back(glm::vec3(0.8f, 1.0f, 0.6f));
            } else {    // red grunts
                colors.push_back(glm::vec3(1.0f, Mth::rand01() * 0.3f, Mth::rand01() * 0.3f));
            }

            entities.push_back(e);
            //cols.push_back(glm::vec3(Mth::rand01(), Mth::rand01(), Mth::rand01()));
        }
        Graphics::setColors(dudeMesh, colors);

        colors.clear();

        for (int i = 0; i < MAX_PROJECTILES; i++) {
            colors.push_back(glm::vec3(1.0f, 0.2f, 0.0f));
        }

        Graphics::setColors(projectileMesh, colors);

        // init particles
        particles.clear();
        for (int i = 0; i < MAX_PARTICLES; i++) {
            Particle p;
            particles.push_back(p);
        }

    }

    void update(float delta) {
        player->update(delta);

        std::vector<glm::mat4> models;
        for (int i = 0; i < entities.size(); i++) {
            entities[i]->update(delta);

            // update dudes model
            glm::mat4 model;
            glm::vec3 pos = entities[i]->getTransform()->lpos;
            glm::vec3 scale = entities[i]->getTransform()->scale;
            pos.y += scale.y / 2.0f;
            model = glm::translate(model, pos);
            model = glm::scale(model, scale);
            models.push_back(model);
        }
        Graphics::setModels(dudeMesh, models);
        models.clear();

        // add projectiles
        std::vector<Projectile>& projectiles = player->getProjectiles();
        for (int i = 0; i < projectiles.size(); i++) {
            PhysicsTransform* pt = projectiles[i].getTransform();

            glm::mat4 model;
            glm::vec3 pos = pt->getPos();
            glm::vec3 scale = pt->scale;
            model = glm::translate(model, pos);
            model = glm::scale(model, scale);
            models.push_back(model);
            for (int j = 0; j < 2; j++)
                SpawnParticle(pt->getPos(), Particle::FIRE, 5);//,projectiles[j].getTransform()->vel);
            for (int j = 0; j < 3; j++)
                SpawnParticle(pt->getPos(), Particle::SPARK, 5);//,projectiles[j].getTransform()->vel);
        }

        Graphics::setModels(projectileMesh, models);
        models.clear();

        // update and build particles
        // should probably remove this into seperate class..
        std::vector<glm::vec3> colors;
        for (int i = 0; i < particles.size(); i++) {
            Particle* p = &particles[i];

            p->update(delta);

            if (p->getTransform()->alive) {

                glm::mat4 model;
                PhysicsTransform* pt = p->getTransform();
                glm::vec3 pos = pt->lpos;
                glm::vec3 scale = pt->scale;
                pos.y += scale.y / 2.0f;
                model = glm::translate(model, pos);
                model = glm::scale(model, scale);
                models.push_back(model);

                if (p->effect == Particle::FIRE) {
                    colors.push_back(glm::vec3(1.0f, p->lifetime / p->maxlifetime, 0.0f));
                } else {
                    colors.push_back(glm::vec3(1.0f));
                }

            }
        }

        Graphics::setModels(particleMesh, models);
        Graphics::setColors(particleMesh, colors);
    }


    void deleteEntities() {
        for (int i = 0; i < entities.size(); i++) {
            delete entities[i];
        }
        entities.erase(entities.begin(), entities.end());
    }
    //void deleteEntities(int startIndex) {
    //    if (startIndex < entities.size()) {
    //        for (int i = startIndex; i < entities.size(); i++) {
    //            delete entities[i];
    //        }
    //        entities.erase(entities.begin() + startIndex, entities.end());
    //    }
    //}


    int curParticle = 0;
    Particle *getNextParticle() {
        Particle *p = &particles[curParticle];
        curParticle = (++curParticle) % (MAX_PARTICLES - 1);
        return p;
    }

    void SpawnParticle(glm::vec3 pos, int effect = Particle::SPARK, float randvel = 0, glm::vec3 vel = glm::vec3(0, 0, 0)) {
        Particle* p = getNextParticle();
        p->effect = effect;
        PhysicsTransform* pt = p->getTransform();
        pt->lpos = pos;
        pt->scale = glm::vec3(.5f, .5f, .5f);
        pt->vel = vel + glm::vec3(vel.x + ((float)rand() / RAND_MAX - .5f) * 2 * randvel, vel.y + ((float)rand() / RAND_MAX - .5f) * 2 * randvel, vel.z + ((float)rand() / RAND_MAX - .5f) * 2 * randvel);//TODO: clean up and have a better randomizer (to be spherical instead of cubic...al)
        p->activate();
    }

    ~EntityManager() {
        deleteEntities();
    }

    GLuint dudeMesh;
private:
    GLuint projectileMesh;

    GLuint particleMesh;
    GLuint particleTex;

    std::vector<Entity*> entities;
    std::vector<Particle> particles;
    Player* player;
};
