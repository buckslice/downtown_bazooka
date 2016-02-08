#pragma once
#include <glm/glm.hpp>
#include "aabb.h"

class Transform {
public:
    glm::vec3 lpos;  //local pos
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
    // should be stored in entity but temporary fix
    // until i get physicsTransform checking in/out better
    bool alive = true;
    float gravityMultiplier = 1.0f;

    // default is 1x1x1 box centered on bottom
    PhysicsTransform(glm::vec3 scale) :
        extmin(glm::vec3(-0.5f, 0.0f, -0.5f)*scale),
        extmax(glm::vec3(0.5f, 1.0f, 0.5f)*scale) {
    }

    AABB getAABB();
    AABB getSwept(float delta);    // uses vel

	inline bool getAffectedByGravity(){return gravityMultiplier != 0.0f;}

private:
    // aabb extents
    glm::vec3 extmin;   // (-0.5f, 0.0f, -0.5f)
    glm::vec3 extmax;   // (0.5f, 2.0f, 0.5f)

};

// class in planning to replace PhysicsTransform
class Collider {
public:
    Transform parent;

    // when register collider just specify whether static or dynamic
    // and it gets put in seperate lists
    //bool isStatic;


private:
    glm::vec3 min;
    glm::vec3 max;

};