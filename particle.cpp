#include "particle.h"

Particle::Particle() : Entity() {
    curlife = 0;
    collider->setExtents(glm::vec3(-0.5f), glm::vec3(0.5f));
}

void Particle::activate(ParticleType type, glm::vec3 pos, glm::vec3 vel,
    float rmag, glm::vec3 scale, bool hasCollision){

    this->type = type;
    transform->setPos(pos);
    transform->setVisibility(Visibility::SHOW_SELF);
    transform->shape = Shape::CUBE_SOLID;
    startScale = scale;
    transform->setScale(startScale);

    collider->enabled = true;
    collider->type = hasCollision ? ColliderType::BASIC : ColliderType::NOCLIP;
    if (rmag > 0.0f) {   // add some random velocity deviation if requested
        collider->vel = vel + Mth::randInsideSphere(1.0f) * rmag;
    } else {
        collider->vel = vel;
    }

    float gravmult;
    switch (type) {
    case SPARK:
        transform->color = glm::vec3(1.0f);
        gravmult = 5.0f;
        lifetime = 1.0f;
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
    case BEACON:
        gravmult = -10.0f;
        lifetime = 2.0f;
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
        transform->setVisibility(Visibility::HIDE_ALL);
        return;
    }

    switch (type) {
    case SPARK:
        collider->vel *= curlife + (1.0f - curlife) * 0.75f;
        transform->setScale(curlife / lifetime * startScale);
        break;
    case FIRE:
        transform->color = glm::vec3(1.0f, curlife / lifetime, 0.0f);
        transform->setScale(curlife / lifetime * startScale);
        collider->vel *= .95f;
        break;
    case BEAM:
        transform->color = HSBColor(curlife / lifetime * 0.1666f + 0.666f, 1.0f, 1.0f).toRGB();
        transform->setScale(curlife / lifetime * startScale);
        collider->vel *= curlife + (1.0f - curlife) * 0.75f;
        break;
    case HEAL: {
        float f = (1.0f - curlife / lifetime) * 0.5f;
        transform->color = glm::vec3(f, 1.0f, f);
        transform->setScale(curlife / lifetime * startScale);
    }break;
    case BEACON:
        transform->color = glm::vec3(curlife / lifetime, 1.0f, 0.0f);
        break;
    }

}
