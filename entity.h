#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "transform.h"
#include "physics.h"

const int MAX_COMPONENTS = 10;

class Entity {
public:
    //glm::vec3 color;
    //bool alive = true;

    Entity(glm::vec3 pos = glm::vec3(0.0f), glm::vec3 scale = glm::vec3(1.0f), glm::vec3 vel = glm::vec3(0.0f));

    // derived classes have to override this
    void virtual update(GLfloat delta) = 0;

    PhysicsTransform* getTransform();

    void setPosition(glm::vec3 pos);

    //long long id;

    /*void setAlive(bool value) {
        alive = value;
        if (alive) {
            transform = Physics::registerDynamic(scale);
        }else{
            Physics::releaseDynamic() // something like this   
        }
    }*/

	void virtual onCollision(){}

    ~Entity() {

    }

    int transform;

private:
    //int components[MAX_COMPONENTS];

};
