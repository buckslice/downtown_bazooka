#include "entity.h"
#include "graphics.h"


Entity::Entity(glm::vec3 pos, glm::vec3 extents, glm::vec3 vel) {
    transform = Graphics::registerTransform();
    collider = Physics::registerDynamic(transform);

    getTransform()->setPos(pos);
    Collider* c = getCollider();
    c->setExtents(extents);
    c->vel = glm::vec3(vel);
}

Transform* Entity::getTransform() {
    return Graphics::getTransform(transform);
}

Collider* Entity::getCollider() {
    return Physics::getCollider(collider);
}
