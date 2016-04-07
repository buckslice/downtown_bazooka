#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "transform.h"
#include "collider.h"

class Entity;

struct CollisionData {
    ColliderType type;  // collider type
    Tag tag;    // collider tag
    Entity* entity;     // pointer to entity you collided with
};

class Entity {
public:

    Entity(glm::vec3 pos = glm::vec3(0.0f), glm::vec3 extents = glm::vec3(1.0f), glm::vec3 vel = glm::vec3(0.0f));

    ~Entity();

    // derived classes have to override this
    virtual void update(GLfloat delta) = 0;

    // may override this if you want
    virtual void onCollision(CollisionData data) {}

    Transform* getTransform();
    Collider* getCollider();

    int transform, collider;

private:

};