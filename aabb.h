#pragma once

#include <glm/glm.hpp>
#include "glm/gtx/component_wise.hpp"
#include "intersectData.h"

class AABB {
public:
    glm::vec3 min;
    glm::vec3 max;

    AABB();

    AABB(const glm::vec3& min, const glm::vec3& max) :
        min(min),
        max(max) {
    }

    //IntersectData getIntersect(const AABB& other) const;

    static bool check(const AABB& a, const AABB& b);

    static AABB getSwept(const AABB& b, const glm::vec3& vel);

    static float sweepTest(const AABB& b1, const AABB& b2, glm::vec3 vel, glm::vec3& norm);

};
