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


ModelEntity::ModelEntity() {
    models = new std::vector<Transform*>();
}


ModelEntity::~ModelEntity() {
    auto& mref = *models;
    for (size_t i = 0, len = mref.size(); i < len; ++i) {
        Graphics::returnTransform(mref[i]);
    }
    delete models;
}

Transform* ModelEntity::registerModel() {
    Transform* t = Graphics::registerTransform();
    models->push_back(t);
    return t;
}