#include "entity.h"


Entity::Entity(glm::vec3 pos, glm::vec3 scale, glm::vec3 vel){
    transform = Physics::registerDynamic(scale);
    PhysicsTransform* pt = getTransform();
    pt->lpos = pos;
    pt->scale = scale;
    pt->vel = vel;
}

PhysicsTransform* Entity::getTransform() {
    return Physics::getTransform(transform);
    // component[0]
}

void Entity::setPosition(glm::vec3 pos) {
    getTransform()->lpos = pos;
}
