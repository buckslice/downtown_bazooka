#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "transform.h"
#include "physics.h"
#include "entity.h"

class Particle : public Entity {
public:
    static const int SPARK = 0;
    static const int CLOUD = 1;
    static const int FIRE = 2;

    float maxlifetime, lifetime;
    int effect;
    //DO NOT CREATE A NEW PARTICLE outside of EntityManager
    Particle() : Entity() {
        getTransform()->alive = false;
        lifetime = 0;
    }

    void activate() {
        getTransform()->alive = true;

        float gravmult;
        switch (effect) {
        case SPARK:
            gravmult = 1.0f;
            maxlifetime = 3.0f;
            break;
        case CLOUD:
            gravmult = -.25f;
            maxlifetime = .5f;
            break;
        case FIRE:
            gravmult = -.5f;
            maxlifetime = 2.0f;
            break;
        }
        this->getTransform()->gravityMultiplier = gravmult;
        lifetime = maxlifetime;
    }

    void update(GLfloat dt) override {
        PhysicsTransform* pt = getTransform();

        if ((lifetime -= dt) <= 0) {
            pt->alive = false;
            return;
        }

        float scalemult;
        switch (effect) {
        case SPARK:
            scalemult = .97f;
            break;
        case CLOUD:
            if (lifetime > maxlifetime*.25f)
                scalemult = 1.02f;
            else
                scalemult = .9f;
            break;
        case FIRE:
            scalemult = .95f;
            break;
        }
        //pt->scale *= scalemult;
        pt->scale = (lifetime / maxlifetime) * glm::vec3(1.0f);
    }
};
