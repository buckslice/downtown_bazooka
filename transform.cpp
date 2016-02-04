
#include "transform.h"
#include "physics.h"

// returns world pos of transform
glm::vec3 Transform::getPos() {
    if (parent >= 0) {
        return Physics::getTransform(parent)->getPos() + lpos;
    }
    return lpos;
}

// set world pos (updates local pos based on parent stuff)

AABB PhysicsTransform::getAABB() {
    return AABB(glm::vec3(lpos + extmin), glm::vec3(lpos + extmax));
}

AABB PhysicsTransform::getSwept(float delta) {
    return AABB::getSwept(getAABB(), vel * delta);
}