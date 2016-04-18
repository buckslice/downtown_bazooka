#include "entity.h"
#include "graphics.h"
#include "physics.h"


Entity::Entity(glm::vec3 pos, glm::vec3 vel) {
    transform = Graphics::registerTransform();
    collider = Physics::registerDynamic(transform);

    transform->setPos(pos);

    collider->vel = glm::vec3(vel);
}

Entity::~Entity() {
    Graphics::returnTransform(transform);
    Physics::returnDynamic(collider);
}
