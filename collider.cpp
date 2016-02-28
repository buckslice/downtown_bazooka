#include "collider.h"

Collider::Collider() {
    type = BASIC;
    setExtents(glm::vec3(-0.5f, 0.0f, -0.5f), glm::vec3(0.5f, 1.0f, 0.5f));
    onCollisionCallback = nullptr;
}

AABB Collider::getAABB() {
    return AABB(glm::vec3(pos + extmin), glm::vec3(pos + extmax));
}

AABB Collider::getSwept(float delta) {
    return AABB::getSwept(getAABB(), vel * delta);
}

void Collider::setExtents(glm::vec3 min, glm::vec3 max) {
    extmin = min;
    extmax = max;
}