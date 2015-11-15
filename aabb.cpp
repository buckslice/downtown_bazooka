#include "aabb.h"

AABB::AABB() {}

bool AABB::check(const AABB& a, const AABB& b) {
    return glm::compMax(glm::max(a.min - b.max, b.min - a.max)) < 0;
}

AABB AABB::getSwept(const AABB& b, const glm::vec3& vel) {
    AABB swept;
    swept.min.x = vel.x > 0 ? b.min.x : b.min.x + vel.x;
    swept.min.y = vel.y > 0 ? b.min.y : b.min.y + vel.y;
    swept.min.z = vel.z > 0 ? b.min.z : b.min.z + vel.z;

    swept.max.x = vel.x > 0 ? b.max.x + vel.x : b.max.x;
    swept.max.y = vel.y > 0 ? b.max.y + vel.y : b.max.y;
    swept.max.z = vel.z > 0 ? b.max.z + vel.z : b.max.z;
    return swept;
}