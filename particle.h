#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "physics.h"
#include "entity.h"
enum ParticleType {
	SPARK,
	FIRE,
	BEAM,
    HEAL,
    BEACON
};
class Particle : public Entity {
public:
    Particle();

	void activate(ParticleType effect, glm::vec3 pos, glm::vec3 vel, 
        float rmag, glm::vec3 scale, bool hasCollision);

	void update(GLfloat dt) override;

    glm::vec3 startScale;
private:

    ParticleType type;
    float lifetime;
    float curlife;

};
