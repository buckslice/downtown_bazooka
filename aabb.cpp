#include "aabb.h"
#include <glm/gtc/matrix_transform.hpp>

AABB::AABB() {}

bool AABB::check(const AABB& a, const AABB& b) {
    //return glm::compMax(glm::max(a.min - b.max, b.min - a.max)) < 0;

    return
        a.min.x <= b.max.x &&
        a.min.y <= b.max.y &&
        a.min.z <= b.max.z &&
        a.max.x >= b.min.x &&
        a.max.y >= b.min.y &&
        a.max.z >= b.min.z;
}

glm::vec3 AABB::getCenter() {
    return min + (max - min) / 2.0f;
}

glm::vec3 AABB::getSize() {
    return max - min;
}

glm::mat4 AABB::getModelMatrix() {
    glm::mat4 model;
    model = glm::translate(model, getCenter());
    model = glm::scale(model, getSize());
    return model;
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

float AABB::sweepTest(const AABB& b1, const AABB& b2, glm::vec3 vel, glm::vec3& norm) {
    // find distance between objects on near and far sides
    glm::vec3 invEntr;
    glm::vec3 invExit;

    if (vel.x > 0.0f) {
        invEntr.x = b2.min.x - b1.max.x;
        invExit.x = b2.max.x - b1.min.x;
    } else {
        invEntr.x = b2.max.x - b1.min.x;
        invExit.x = b2.min.x - b1.max.x;
    }
    if (vel.y > 0.0f) {
        invEntr.y = b2.min.y - b1.max.y;
        invExit.y = b2.max.y - b1.min.y;
    } else {
        invEntr.y = b2.max.y - b1.min.y;
        invExit.y = b2.min.y - b1.max.y;
    }
    if (vel.z > 0.0f) {
        invEntr.z = b2.min.z - b1.max.z;
        invExit.z = b2.max.z - b1.min.z;
    } else {
        invEntr.z = b2.max.z - b1.min.z;
        invExit.z = b2.min.z - b1.max.z;
    }

    // find time of collision
    glm::vec3 entr;
    glm::vec3 exit;

    if (vel.x == 0.0f) {
        entr.x = -std::numeric_limits<float>::infinity();
        exit.x = std::numeric_limits<float>::infinity();
    } else {
        entr.x = invEntr.x / vel.x;
        exit.x = invExit.x / vel.x;
    }
    if (vel.y == 0.0f) {
        entr.y = -std::numeric_limits<float>::infinity();
        exit.y = std::numeric_limits<float>::infinity();
    } else {
        entr.y = invEntr.y / vel.y;
        exit.y = invExit.y / vel.y;
    }
    if (vel.z == 0.0f) {
        entr.z = -std::numeric_limits<float>::infinity();
        exit.z = std::numeric_limits<float>::infinity();
    } else {
        entr.z = invEntr.z / vel.z;
        exit.z = invExit.z / vel.z;
    }

    float entrTime = glm::compMax(entr);
    float exitTime = glm::compMin(exit);

    // if there was no collision
    if (entrTime > exitTime ||
        entr.x < 0.0f && entr.y < 0.0f && entr.z < 0.0f ||
        entr.x > 1.0f || entr.y > 1.0f || entr.z > 1.0f) {
        norm = glm::vec3(0.0f);
        return 1.0f;
    }

    if (entr.x > entr.y) {
        if (entr.x > entr.z) {
            norm = glm::vec3(invEntr.x < 0.0f ? 1.0f : -1.0f, 0.0f, 0.0f);
        } else {
            norm = glm::vec3(0.0f, 0.0f, invEntr.z < 0.0f ? 1.0f : -1.0f);
        }
    } else {
        if (entr.y > entr.z) {
            norm = glm::vec3(0.0f, invEntr.y < 0.0f ? 1.0f : -1.0f, 0.0f);
        } else {
            norm = glm::vec3(0.0f, 0.0f, invEntr.z < 0.0f ? 1.0f : -1.0f);
        }
    }

    return entrTime;
}

float AABB::sweepTest2(const AABB& a, const AABB& b, glm::vec3 vel, glm::vec3& norm) {
    float hitTime = 0.0f;
    float outTime = 1.0f;
    glm::vec3 overlapTime = glm::vec3(0.0f);
    vel = -vel;

    if (vel.x < 0.0f) {
        if (b.max.x < a.min.x) return 1.0f;
        if (b.max.x > a.min.x) outTime = glm::min((a.min.x - b.max.x) / vel.x, outTime);
        if (a.max.x < b.min.x) {
            overlapTime.x = (a.max.x - b.min.x) / vel.x;
            hitTime = glm::max(overlapTime.x, hitTime);
        }
    } else if (vel.x > 0.0f) {
        if (b.min.x > a.max.x) return 1.0f;
        if (a.max.x > b.min.x) outTime = glm::min((a.max.x - b.min.x) / vel.x, outTime);
        if (b.max.x < a.min.x) {
            overlapTime.x = (a.min.x - b.max.x) / vel.x;
            hitTime = glm::max(overlapTime.x, hitTime);
        }
    }

    if (hitTime > outTime) return 1.0f;

    if (vel.y < 0.0f) {
        if (b.max.y < a.min.y) return 1.0f;
        if (b.max.y > a.min.y) outTime = glm::min((a.min.y - b.max.y) / vel.y, outTime);
        if (a.max.y < b.min.y) {
            overlapTime.y = (a.max.y - b.min.y) / vel.y;
            hitTime = glm::max(overlapTime.y, hitTime);
        }
    } else if (vel.y > 0.0f) {
        if (b.min.y > a.max.y) return 1.0f;
        if (a.max.y > b.min.y) outTime = glm::min((a.max.y - b.min.y) / vel.y, outTime);
        if (b.max.y < a.min.y) {
            overlapTime.y = (a.min.y - b.max.y) / vel.y;
            hitTime = glm::max(overlapTime.y, hitTime);
        }
    }

    if (hitTime > outTime) return 1.0f;

    if (vel.z < 0.0f) {
        if (b.max.z < a.min.z) return 1.0f;
        if (b.max.z > a.min.z) outTime = glm::min((a.min.z - b.max.z) / vel.z, outTime);
        if (a.max.z < b.min.z) {
            overlapTime.z = (a.max.z - b.min.z) / vel.z;
            hitTime = glm::max(overlapTime.z, hitTime);
        }
    } else if (vel.z > 0.0f) {
        if (b.min.z > a.max.z) return 1.0f;
        if (a.max.z > b.min.z) outTime = glm::min((a.max.z - b.min.z) / vel.z, outTime);
        if (b.max.z < a.min.z) {
            overlapTime.z = (a.min.z - b.max.z) / vel.z;
            hitTime = glm::max(overlapTime.z, hitTime);
        }
    }

    if (hitTime > outTime) return 1.0f;

    if (overlapTime.x > overlapTime.y) {
        if (overlapTime.x > overlapTime.z) {
            norm = glm::vec3(glm::sign(vel.x), 0.0f, 0.0f);
        } else {
            norm = glm::vec3(0.0f, 0.0f, glm::sign(vel.z));
        }
    } else {
        if (overlapTime.y > overlapTime.z) {
            norm = glm::vec3(0.0f, glm::sign(vel.y), 0.0f);
        } else {
            norm = glm::vec3(0.0f, 0.0f, glm::sign(vel.z));
        }
    }

    return hitTime;
}

float AABB::sweepTest3(const AABB& a, const AABB& b, glm::vec3 v, glm::vec3& n) {
    glm::vec3 f = glm::vec3(1000.0f);
    glm::vec3 l = glm::vec3(-1000.0f);

    v = -v;
    for (int i = 0; i < 3; ++i) {
        if (v[i] == 0) {
            f[i] = 0;
            l[i] = 1;
            continue;
        }

        if (a.max[i] < b.min[i] && v[i] < 0)
            f[i] = (a.max[i] - b.min[i]) / v[i];

        else if (b.max[i] < a.min[i] && v[i] > 0)
            f[i] = (a.min[i] - b.max[i]) / v[i];

        if (b.max[i] > a.min[i] && v[i] < 0)
            l[i] = (a.min[i] - b.max[i]) / v[i];

        else if (a.max[i] > b.min[i] && v[i] > 0)
            l[i] = (a.max[i] - b.min[i]) / v[i];
    }

    float first = glm::compMax(f);
    float last = glm::compMax(l);

    if (first >= 0 && last <= 1 && first <= last) {

    } else {
        return 1.0f;
    }

    for (size_t i = 0; i < 3; ++i) {
        if (f[i] == first) {
            n[i] = v[i] > 0 ? -1.f : 1.f;
            break;
        }
    }

    

    if (first > 1.0f) {
        first = 1.0f;
    } else if (first < 0.0f) {
        first = 0.0f;
    }

    return first;
}

