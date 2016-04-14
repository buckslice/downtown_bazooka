#include "entity.h"
#include "graphics.h"
#include "physics.h"


Entity::Entity(glm::vec3 pos, glm::vec3 extents, glm::vec3 vel) {
    transform = Graphics::registerTransform();
    collider = Physics::registerDynamic(transform);

    transform->setPos(pos);

    // defaults to 1x1x1 box scaled by extents with origin at bottom center
    glm::vec3 min = glm::vec3(-0.5f, 0.0f, -0.5f)*extents;
    glm::vec3 max = glm::vec3(0.5f, 1.0f, 0.5f)*extents;
    collider->setExtents(min, max);
    collider->vel = glm::vec3(vel);
}

Entity::~Entity() {
    Graphics::returnTransform(transform);
    Physics::returnDynamic(collider);
}
