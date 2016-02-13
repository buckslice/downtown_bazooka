#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "transform.h"
#include "physics.h"

class Entity {
public:
    //glm::vec3 color;
    //bool alive = true;

    Entity(glm::vec3 pos = glm::vec3(0.0f), glm::vec3 scale = glm::vec3(1.0f), glm::vec3 vel = glm::vec3(0.0f));
    ~Entity() {
        
    }

    // derived classes have to override this
    void virtual update(GLfloat delta) = 0;

    PhysicsTransform* getTransform();

	void virtual onCollision(){}    // TODO

    int transform;

private:

};
