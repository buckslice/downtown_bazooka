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
    BEACON,
    BEACON_TRIGGERED,
    BOOST
};
class Particle : public Entity {
public:
    Particle();

	void activate(ParticleType effect, glm::vec3 pos, glm::vec3 vel, 
        float rmag, glm::vec3 scale, bool hasCollision);

	void update(GLfloat dt) override;

private:
    glm::vec3 startScale;
    ParticleType type;
    float lifetime;
    float curlife;

};
