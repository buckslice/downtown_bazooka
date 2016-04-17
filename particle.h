#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "physics.h"
#include "entity.h"
enum ParticleType {
	SPARK,
	CLOUD,
	FIRE,
	BEAM,
    HEAL   
};
class Particle : public Entity {
public:

    float lifetime, curlife;
	ParticleType type;

    Particle();

	void activate();

	void update(GLfloat dt) override;

	glm::vec3 getColor();
};
