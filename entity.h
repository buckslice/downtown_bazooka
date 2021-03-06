#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "transform.h"
#include "collider.h"


class Entity {
public:

    Entity(glm::vec3 pos = glm::vec3(0.0f), glm::vec3 vel = glm::vec3(0.0f));

    ~Entity();

    // derived classes have to override this
    virtual void update(GLfloat delta) = 0;

    // may override this if you want
    virtual void onCollision(Tag tag, Entity* other = nullptr) {}

    Transform* transform;
    Collider* collider;

    // delete copy constructor so pools dont get messed up
    //Entity(const Entity& other) = delete;
private:

};


class ModelEntity : public Entity {
public:
    ModelEntity();
    ~ModelEntity();
protected:
    // returns a new model transform that will automatically
    // get returned when this class instance is destructed
    Transform* registerModel();
private:
    std::vector<Transform*>* models;
};