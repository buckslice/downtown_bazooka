#include "Particle.h"

void Particle::activate() {
    transform = Physics::registerDynamic();

    float gravmult;
    switch (effect) {
    case SPARK:
        gravmult = 5.0f;
        lifetime = 1.0f;
        break;
    case CLOUD:
        gravmult = -.25f;
        lifetime = .5f;
        break;
    case FIRE:
        gravmult = -.5f;
        lifetime = 2.0f;
        break;
    }
    this->getTransform()->gravityMultiplier = gravmult;
    curlife = lifetime;
}

void Particle::update(GLfloat dt) {
    PhysicsTransform* pt = getTransform();

    if (pt != nullptr && (curlife -= dt) <= 0) {
        Physics::returnDynamic(transform);
        return;
    }

    float scalemult;
    switch (effect) {
    case SPARK:
        scalemult = .97f;
        break;
    case CLOUD:
        if (curlife > lifetime*.25f)
            scalemult = 1.02f;
        else
            scalemult = .9f;
        break;
    case FIRE:
        scalemult = .95f;
        break;
    }
    //pt->scale *= scalemult;
    pt->setScale(curlife / lifetime * glm::vec3(1.0f));
}

glm::vec3 Particle::getColor() {
    switch (effect) {
    case FIRE:
        return glm::vec3(1.0f, curlife / lifetime, 0.0f);
    }
    return glm::vec3(1.0f);
}
