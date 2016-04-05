#include "particle.h"

Particle::Particle() : Entity() {
    curlife = 0;
    getCollider()->setExtents(glm::vec3(-0.5f), glm::vec3(0.5f));
}

void Particle::activate() {
    Transform* t = getTransform();
    t->setVisibility(VISIBLE);
    t->solid = true;

    Collider* c = getCollider();
    c->enabled = true;

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
        gravmult = Mth::rand0X(.15f) - .5f;
        lifetime = 2.0f;
        break;
	case BEAM:
		gravmult = 0.0f;
		lifetime = 0.5f;
    }
    c->gravityMultiplier = gravmult;
    curlife = lifetime;
}

void Particle::update(GLfloat dt) {
    Collider* c = getCollider();
    if (!c->enabled) {
        return;
    } else if ((curlife -= dt) <= 0) {
        c->enabled = false;
        getTransform()->setVisibility(HIDDEN);
        return;
    }

	// scalemult unused
    float scalemult;
    switch (type) {
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
		getCollider()->vel *= .95f;
        scalemult = .95f;
        break;
	case BEAM:
		scalemult = 1.02f;
		getCollider()->vel *= curlife + (1.0f - curlife) * 0.75f;
		break;
    }

    Transform* t = getTransform();
    t->color = getColor();
    t->setScale(curlife / lifetime * glm::vec3(1.0f));
}

glm::vec3 Particle::getColor() {
    switch (type) {
    case FIRE:
        return glm::vec3(1.0f, curlife / lifetime, 0.0f);
	case BEAM:
		return HSBColor(curlife / lifetime * 0.1666f + 0.666f, 1.0f, 1.0f).toRGB();
    }
    return glm::vec3(1.0f);
}
