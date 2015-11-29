#pragma once
#include <glm/glm.hpp>
#include "aabb.h"

class Transform {
public:
    glm::vec3 pos;  //local pos
    glm::vec3 scale;
    //glm::vec3 rot;

    int parent = -1;

    // returns world pos
    glm::vec3 getPos();

private:


};

// should always be parent transform
class PhysicsTransform : public Transform {
public:
    glm::vec3 vel;
    bool grounded = false;

    // default is 1x1x1 box centered on bottom
    PhysicsTransform(glm::vec3 scale) :
        extmin(glm::vec3(-0.5f, 0.0f, -0.5f)*scale),
        extmax(glm::vec3(0.5f, 1.0f, 0.5f)*scale) {
    }

    AABB getAABB();
    AABB getSwept(float delta);    // uses vel

private:
    // aabb extents
    glm::vec3 extmin;   // (-0.5f, 0.0f, -0.5f)
    glm::vec3 extmax;   // (0.5f, 2.0f, 0.5f)

};