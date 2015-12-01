#pragma once
#include <vector>
#include "entity.h"
#include "enemy.h"
#include "mathutil.h"

class EntityManager {
public:
    EntityManager(Graphics& g, Player* player) :g(g), player(player) {
        entities.push_back(player);
    }

    // should add in support for adding single entities dynamically next
    // main problem is need to redo the graphics to just create buffer with fixed MAX_ENTITIES size
    // then update it each time a new entity is spawned using glBufferSubData()
    // or maybe glMapBuffer() or glMapBufferRange()
    void init(int num) {
        deleteEntities(1);  // player is at index 0
        std::vector<glm::vec3> cols;
        for (int i = 0; i < num; i++) {
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
        // push on extra colors at end for projectiles
        for (int i = 0; i < 100; i++) {
            cols.push_back(glm::vec3(0.95f, 1.0f, 0.5f));
        }
        dudeColorBuffer = g.genColorBuffer(*g.guy, cols);
        initColors = true;
    }

    void update(float delta) {
        std::vector<glm::mat4> models;
        for (int i = 0; i < entities.size(); i++) {
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
        }
        if (initModels) {
            glDeleteBuffers(1, &dudeModelBuffer);
			//glDeleteBuffers(1, &projectileModelBuffer);
        }
        dudeModelBuffer = g.genModelBuffer(*g.guy, models);
		//projectileModelBuffer = g.genModelBuffer(*g.guy, projectileModels);
        initModels = true;
    }

    void deleteEntities(int startIndex) {
        if (startIndex < entities.size()) {
            for (int i = startIndex; i < entities.size(); i++) {
                delete entities[i];
            }
            entities.erase(entities.begin() + startIndex, entities.end());
        }
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