#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "physics.h"
#include "entity.h"

class Particle : public Entity {
public:
    static const int SPARK = 0;
    static const int CLOUD = 1;
    static const int FIRE = 2;

    float lifetime, curlife;
    //bool alive;
    int effect;

    Particle() : Entity() {
        curlife = 0;
    }

	void activate();

	void update(GLfloat dt) override;

	glm::vec3 getColor();
};
