#include "entity.h"
#include "graphics.h"


Entity::Entity(glm::vec3 pos, glm::vec3 extents, glm::vec3 vel) {
    transform = Graphics::registerTransform();
    collider = Physics::registerDynamic(transform);

    getTransform()->setPos(pos);
    Collider* c = getCollider();

    // defaults to 1x1x1 box scaled by extents with origin at bottom center
    glm::vec3 min = glm::vec3(-0.5f, 0.0f, -0.5f)*extents;
    glm::vec3 max = glm::vec3(0.5f, 1.0f, 0.5f)*extents;
    c->setExtents(min, max);

    c->vel = glm::vec3(vel);
}

Entity::~Entity() {
}

Transform* Entity::getTransform() {
    return Graphics::getTransform(transform);
}

Collider* Entity::getCollider() {
    return Physics::getCollider(collider);
}
