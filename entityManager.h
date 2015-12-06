#pragma once
#include <vector>
#include "entity.h"
#include "enemy.h"
#include "mathutil.h"
#include "Particle.h"

class EntityManager {
public:
    //EntityManager(Graphics& g, Player* player);
	
	EntityManager(Graphics& g, Player* player) :g(g), player(player) {
		entities.push_back(player);
	}

	int ENEMIES_START,ENEMIES_END;

	int PROJECTILES_START,PROJECTILES_END;
	const int PROJECTILES_MAX = 100;

	int PARTICLES_START,PARTICLES_END;
	const int PARTICLES_MAX = 1000;

    // should add in support for adding single entities dynamically next
    // main problem is need to redo the graphics to just create buffer with fixed MAX_ENTITIES size
    // then update it each time a new entity is spawned using glBufferSubData()
    // or maybe glMapBuffer() or glMapBufferRange()
	std::vector<glm::vec3> cols;
    void init(int numenemies) {
        deleteEntities(1);  // player is at index 0

		ENEMIES_START = 1;
		ENEMIES_END = ENEMIES_START+numenemies;
        for (int i = 0; i < numenemies; i++) {
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
                cols.push_back(glm::vec3(0.8f, 1.0f, 0.6f));
            } else {    // red grunts
                cols.push_back(glm::vec3(1.0f, Mth::rand01() * 0.3f, Mth::rand01() * 0.3f));
            }

            entities.push_back(e);
            //cols.push_back(glm::vec3(Mth::rand01(), Mth::rand01(), Mth::rand01()));
        }
        if (initColors) {
            glDeleteBuffers(1, &dudeColorBuffer);
        }

		// put stuff for particles (colors may be changed when the particle is actually created)
		PARTICLES_START = ENEMIES_END+1;
		PARTICLES_END = PARTICLES_START+PARTICLES_MAX;
		for(int i = 0; i < PARTICLES_MAX; i++){
			cols.push_back(glm::vec3(1.0f,1.0f,1.0f));
			Particle* particle = new Particle();
			entities.push_back(particle);
		}

        // push on extra colors at end for projectiles
		PROJECTILES_START = PARTICLES_END+1;
		PROJECTILES_END = PROJECTILES_START+PROJECTILES_MAX;
        for (int i = 0; i < PROJECTILES_MAX; i++) {
            cols.push_back(glm::vec3(0.95f, 1.0f, 0.5f));
        }

        initColors = true;
        dudeColorBuffer = g.genColorBuffer(*g.guy, cols);
    }

    void update(float delta) {
        std::vector<glm::mat4> models;
		int colorsindex = 0;
        for (int i = 0; i < entities.size(); i++) {
			if (i >= PARTICLES_START && i <= PARTICLES_END){
				Particle *particle = (Particle*)entities[i];
				if(!particle->alive)
					continue;
				if (particle->effect == Particle::FIRE) {
					cols[colorsindex] = glm::vec3(1.0f,particle->lifetime/particle->maxlifetime,0.0f);
				}
			}

            entities[i]->update(delta);
            if (i == 0) {   // player is always first
				 continue;
            }
            // update dudes model
            glm::mat4 model;
            glm::vec3 pos = entities[i]->getTransform()->pos;
            glm::vec3 scale = entities[i]->getTransform()->scale;
            pos.y += scale.y / 2.0f;
            model = glm::translate(model, pos);
            model = glm::scale(model, scale);
            models.push_back(model);
			++colorsindex;
        }

        // add projectiles onto end
        std::vector<Projectile>& projectiles = player->getProjectiles();
        for (int j = 0; j < projectiles.size(); j++) {
            glm::mat4 model;
            glm::vec3 pos = projectiles[j].getTransform()->pos;
            glm::vec3 scale = projectiles[j].getTransform()->scale;
            model = glm::translate(model, pos);
            model = glm::scale(model, scale);
            models.push_back(model);
			for(int k = 0; k < 2; k++)
				SpawnParticle(projectiles[j].getTransform()->getPos(),Particle::FIRE,5);//,projectiles[j].getTransform()->vel);
			for(int k = 0; k < 3; k++)
				SpawnParticle(projectiles[j].getTransform()->getPos(),Particle::SPARK,5);//,projectiles[j].getTransform()->vel);
        }
        if (initModels) {
            glDeleteBuffers(1, &dudeModelBuffer);
			//glDeleteBuffers(1, &projectileModelBuffer);
        }
        dudeModelBuffer = g.genModelBuffer(*g.guy, models);
		//projectileModelBuffer = g.genModelBuffer(*g.guy, projectileModels);
        initModels = true;
        dudeColorBuffer = g.genColorBuffer(*g.guy, cols);
		initColors = true;
    }

    void deleteEntities(int startIndex) {
        if (startIndex < entities.size()) {
            for (int i = startIndex; i < entities.size(); i++) {
                delete entities[i];
            }
            entities.erase(entities.begin() + startIndex, entities.end());
        }
    }

	int curParticle;
	Particle *getNextParticle(){
		Particle *p = (Particle*)entities.at(PARTICLES_START+curParticle);
		curParticle = (++curParticle)%(PARTICLES_MAX-1);
		return p;
	}

	void SpawnParticle(glm::vec3 pos,int effect = Particle::SPARK,float randvel = 0,glm::vec3 vel = glm::vec3(0,0,0)) {
		Particle* p = getNextParticle();
		p->effect = effect;
		p->setPosition(pos);
		p->getTransform()->scale = glm::vec3(.5f,.5f,.5f);
		p->getTransform()->vel = vel+glm::vec3(vel.x+((float)rand()/RAND_MAX-.5f)*2*randvel,vel.y+((float)rand()/RAND_MAX-.5f)*2*randvel,vel.z+((float)rand()/RAND_MAX-.5f)*2*randvel);//TODO: clean up and have a better randomizer (to be spherical instead of cubic...al)
		p->activate();
	}

    ~EntityManager() {
        deleteEntities(0);
    }

private:
    std::vector<Entity*> entities;
    GLuint dudeColorBuffer;
    GLuint dudeModelBuffer;
	//GLuint projectileModelBuffer;
    Graphics& g;
    Player* player;
    bool initColors = false;
    bool initModels = false;

};

//EntityManager* GetEntityManager(){return (EntityManager*)EntityManagerInstance;}
