
#include "physics.h"
//#include "transform.h"

////returns world pos of transform
//glm::vec3 Transform::getPos() {
//    if (parent >= 0) {
//        return Physics::getTransform(parent)->getPos() + lpos;
//    }
//    return lpos;
//}


// physics stuff
AABB PhysicsTransform::getAABB() {
    return AABB(glm::vec3(getWorldPos() + extmin), glm::vec3(getWorldPos() + extmax));
}

AABB PhysicsTransform::getSwept(float delta) {
    return AABB::getSwept(getAABB(), vel * delta);
}

void PhysicsTransform::setExtents(glm::vec3 scale) {
    extmin = glm::vec3(-0.5f, 0.0f, -0.5f)*scale;
    extmax = glm::vec3(0.5f, 1.0f, 0.5f)*scale;
}