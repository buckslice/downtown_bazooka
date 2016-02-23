#include "particle.h"

Particle::Particle() : Entity() {
    curlife = 0;
    getCollider()->setExtents(glm::vec3(-0.5f), glm::vec3(0.5f));
}

void Particle::activate() {
    Transform* t = getTransform();
    t->visible = true;
    t->solid = true;

    Collider* c = getCollider();
    c->awake = true;

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
        gravmult = Mth::rand0X(.15f) - .5f;
        lifetime = 2.0f;
        break;
    }
    c->gravityMultiplier = gravmult;
    curlife = lifetime;
}

void Particle::update(GLfloat dt) {
    Collider* c = getCollider();
    if (!c->awake) {
        return;
    } else if ((curlife -= dt) <= 0) {
        c->awake = false;
        getTransform()->visible = false;
        return;
    }

    float scalemult;
    switch (effect) {
    case SPARK:
        scalemult = .97f;
        getCollider()->vel *= curlife + (1.0f - curlife) * 0.75f;
        //getCollider()->vel *= .95f;
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

    Transform* t = getTransform();
    t->color = getColor();
    t->setScale(curlife / lifetime * glm::vec3(1.0f));
}

glm::vec3 Particle::getColor() {
    switch (effect) {
    case FIRE:
        return glm::vec3(1.0f, curlife / lifetime, 0.0f);
    }
    return glm::vec3(1.0f);
}
