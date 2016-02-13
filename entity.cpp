#include "entity.h"
#include "graphics.h"


Entity::Entity(glm::vec3 pos, glm::vec3 scale, glm::vec3 vel){
    transform = Physics::registerDynamic();
    PhysicsTransform* pt = getTransform();
    pt->setPos(pos);
    pt->setExtents(scale);
    pt->vel = vel;

    // planned move here
    //int index = Graphics::registerTransform();
    //BTransform* bt = Graphics::getTransform(index);
    //bt->setScale(scale);
    //pt->parentAll(bt);
}

PhysicsTransform* Entity::getTransform() {
    return Physics::getTransform(transform);
}
