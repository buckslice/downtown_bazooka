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

    Entity(glm::vec3 pos = glm::vec3(0.0f), glm::vec3 scale = glm::vec3(1.0f), glm::vec3 vel = glm::vec3(0.0f)) {
        transform = Physics::registerDynamic(scale);
        PhysicsTransform* pt = getTransform();
        pt->lpos = pos;
        pt->scale = scale;
        pt->vel = vel;
    }

    // derived classes have to override this
    void virtual update(GLfloat delta) = 0;

    PhysicsTransform* getTransform() {
        return Physics::getTransform(transform);
        // component[0]
    }

    void setPosition(glm::vec3 pos) {
        getTransform()->lpos = pos;
    }

    int transform;

    //long long id;

    /*void setAlive(bool value) {
        alive = value;
        if (alive) {
            transform = Physics::registerDynamic(scale);
        }else{
            Physics::releaseDynamic() // something like this   
        }
    }*/

    ~Entity() {

    }
private:
    //int components[MAX_COMPONENTS];

};