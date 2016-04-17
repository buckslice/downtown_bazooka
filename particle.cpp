#include "particle.h"

Particle::Particle() : Entity() {
    curlife = 0;
    collider->setExtents(glm::vec3(-0.5f), glm::vec3(0.5f));
}

void Particle::activate() {
    transform->setVisibility(VISIBLE);
    transform->shape = Shape::CUBE_SOLID;

    collider->enabled = true;

    float gravmult;
    switch (type) {
    case SPARK:
        gravmult = 5.0f;
        lifetime = 1.0f;
        break;
    case CLOUD:
        gravmult = -.25f;
        lifetime = .5f;
        break;
    case FIRE:
        gravmult = Mth::randRange(-0.5f, -0.35f);
        lifetime = 2.0f;
        break;
    case BEAM:
        gravmult = 0.0f;
        lifetime = 0.5f;
        break;
    case HEAL:
        gravmult = -1.0f;
        lifetime = 0.5f;
        break;
    }
    collider->gravityMultiplier = gravmult;
    curlife = lifetime;

    update(0);  // incase spawned after the update this frame
}

void Particle::update(GLfloat dt) {
    if (!collider->enabled) {
        return;
    } else if ((curlife -= dt) <= 0) {
        collider->enabled = false;
        transform->setVisibility(HIDDEN);
        return;
    }

    switch (type) {
    case SPARK:
        collider->vel *= curlife + (1.0f - curlife) * 0.75f;
        break;
    case CLOUD:
        break;
    case FIRE:
        collider->vel *= .95f;
        break;
    case BEAM:
        collider->vel *= curlife + (1.0f - curlife) * 0.75f;
        break;
    case HEAL:
        //collider->vel *= curlife;
        break;
    }

    transform->color = getColor();
    transform->setScale(curlife / lifetime * glm::vec3(1.0f));
}

glm::vec3 Particle::getColor() {
    switch (type) {
    case FIRE:
        return glm::vec3(1.0f, curlife / lifetime, 0.0f);
    case BEAM:
        return HSBColor(curlife / lifetime * 0.1666f + 0.666f, 1.0f, 1.0f).toRGB();
    case HEAL:
        float f = (1.0f - curlife / lifetime) * 0.5f;
        return glm::vec3(f, 1.0f, f);
    }
    return glm::vec3(1.0f);
}
