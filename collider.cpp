#include "collider.h"

Collider::Collider() {
    type = ColliderType::BASIC;
    tag = Tag::NONE;
    setExtents(glm::vec3(-0.5f, 0.0f, -0.5f), glm::vec3(0.5f, 1.0f, 0.5f));
}

AABB Collider::getAABB() {
    return AABB(glm::vec3(pos + extmin), glm::vec3(pos + extmax));
}

void Collider::setExtents(glm::vec3 min, glm::vec3 max) {
    extmin = min;
    extmax = max;
}