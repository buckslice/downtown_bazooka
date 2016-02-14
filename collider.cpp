#include "collider.h"

Collider::Collider() {
    setExtents(glm::vec3(1.0f));
}

AABB Collider::getAABB() {
    return AABB(glm::vec3(pos + extmin), glm::vec3(pos + extmax));
}

AABB Collider::getSwept(float delta) {
    return AABB::getSwept(getAABB(), vel * delta);
}

// defaults to 1x1x1 box with origin on bottom center
// should prob just add in method to set min and max explicitly
void Collider::setExtents(glm::vec3 scale) {
    extmin = glm::vec3(-0.5f, 0.0f, -0.5f)*scale;
    extmax = glm::vec3(0.5f, 1.0f, 0.5f)*scale;
}