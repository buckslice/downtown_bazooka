#include "aabb.h"
#include <glm/gtc/matrix_transform.hpp>
#include "glm/gtx/component_wise.hpp"

AABB::AABB() {}

bool AABB::check(const AABB& a, const AABB& b) {
    //return glm::compMax(glm::max(a.min - b.max, b.min - a.max)) < 0;
    return
        a.min.x <= b.max.x && a.min.y <= b.max.y && a.min.z <= b.max.z &&
        a.max.x >= b.min.x && a.max.y >= b.min.y && a.max.z >= b.min.z;
}

glm::vec3 AABB::getCenter() const{
    return min + (max - min) / 2.0f;
}

// getSize().x == width
// .z = length
// .y = height
glm::vec3 AABB::getSize() const{
    return max - min;
}

glm::mat4 AABB::getModelMatrix() const {
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
