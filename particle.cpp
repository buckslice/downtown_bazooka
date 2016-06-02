#include "particle.h"

Particle::Particle() : Entity() {
    curlife = 0;
    collider->setExtents(glm::vec3(-0.5f), glm::vec3(0.5f));
}

void Particle::activate(ParticleType type, glm::vec3 pos, glm::vec3 vel,
    float rmag, glm::vec3 scale, bool hasCollision) {

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
    case BEAM_HOMING:
        gravmult = 0.0f;
        //lifetime = 0.5f;
        lifetime = 2.0f;
        break;
    case HEAL:
        gravmult = -1.0f;
        lifetime = 0.5f;
        break;
    case BEACON:
    case BEACON_TRIGGERED:
        gravmult = -10.0f;
        lifetime = 2.0f;
        break;
    case BOOST:
        gravmult = 0.0f;
        lifetime = 2.0f;
        break;
    case SWITCH:
        gravmult = -1.0f;
        lifetime = 3.0f;
        break;
    }
    collider->gravityMultiplier = gravmult;
    curlife = lifetime;

    update(0);  // incase spawned after the update this frame
}

void Particle::update(GLfloat dt) {
    if (!collider->enabled) {
        return;
    } else if ((curlife -= dt) <= 0.0f) {
        collider->enabled = false;
        transform->setVisibility(Visibility::HIDE_ALL);
        return;
    }

    // precalculate lifetime lerp value for convenience
    // equal to 1.0f at beginning of particle life, 0.0f at end
    float t = curlife / lifetime;

    switch (type) {
    case SPARK:
        collider->vel *= curlife + (1.0f - curlife) * 0.75f;
        break;
    case FIRE:
        transform->color = glm::vec3(1.0f, t, 0.0f);
        collider->vel *= .95f;
        break;
    case BEAM: {
        int ps = (int)(t * 20);
        if (ps % 2 == 0) {
            transform->color = HSBColor(t * 0.1f + 0.666f, 1.0f, 1.0f).toRGB()*0.5f;
        } else {
            transform->color = HSBColor(1.0f, t * 0.1f + 0.666f, 1.0f).toRGB()*0.5f;
        }
        collider->vel += collider->vel*(2.0f - curlife)*dt;
    }break;
    case BEAM_HOMING: {
        int ps = (int)(t * 10);
        if (ps % 2 == 0) {
            transform->color = HSBColor(t * 0.2f + 0.2f, 1.0f, 1.0f).toRGB()*0.5f;
        } else {
            transform->color = HSBColor(t * 0.1f + 0.5f, 1.0f, 1.0f).toRGB()*0.5f;
        }
        collider->vel += collider->vel*(2.0f - curlife)*dt;
    }break;
    case HEAL: {
        float f = (1.0f - t) * 0.5f;
        transform->color = glm::vec3(f, 1.0f, f);
    }break;
    case BEACON:
        transform->color = glm::vec3(t, 1.0f, 0.0f);
        break;
    case BEACON_TRIGGERED:
        transform->color = glm::vec3(t, 0.0f, 1.0f);
        break;
    case BOOST:
        transform->color = Mth::lerp(glm::vec3(1.0f, 0.25f, 0.5f),
            glm::vec3(0.5f, 0.0f, 1.0f)*0.5f, 1.0f - t);
        collider->vel -= collider->vel * Mth::saturate(2.0f * dt);
        break;
    case SWITCH:
        float flicker = abs(sin(t*10.0f));
        transform->color = Mth::lerp(glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(1.0f), flicker);
        break;
    }

    // set scale based on remaining lifetime
    transform->setScale(t * startScale);

}
