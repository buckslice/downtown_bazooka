
#include "transform.h"
#include "physics.h"

// returns world pos of transform
glm::vec3 Transform::getPos() {
    if (parent >= 0) {
        return Physics::getTransform(parent)->pos + pos;
    }
    return pos;
}

AABB PhysicsTransform::getAABB() {
    return AABB(glm::vec3(pos + extmin), glm::vec3(pos + extmax));
}

AABB PhysicsTransform::getSwept(float delta) {
    return AABB::getSwept(getAABB(), vel * delta);
}