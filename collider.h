#pragma once
#include <glm/glm.hpp>
#include "aabb.h"

class Collider {
public:
    int transform;  // id of transform this collider is changing position of

    glm::vec3 vel;
    bool grounded = false;
    bool awake = true;

    float gravityMultiplier = 1.0f;

    Collider();

    void setExtents(glm::vec3 scale);

    AABB getAABB();
    AABB getSwept(float delta);    // uses vel


    //glm::vec3 getPos();

    // when register collider just specify whether static or dynamic
    // and it gets put in seperate lists
    //bool isStatic;

    // maybe make a struct with this and some sort of typed data
    // like int for damage or string for something
    // maybe make it fire an event for event system?
    //enum Type {
    //    PLAYER,
    //    ENTITY,
    //    PARTICLE,
    //    PLAYER_PROJECTILE,
    //    ENEMY_PROJECTILE
    //};

private:
    glm::vec3 extmin;   // (-0.5f, 0.0f, -0.5f)
    glm::vec3 extmax;   // (0.5f, 2.0f, 0.5f)

    // gets set at beginning of each physics iteration
    // could prob make this better somehow (two out of cache jumps per collider per frame (probly (i have no clue))
    glm::vec3 pos; 
    friend class Physics;   // so physics can directly set pos but nobody else
};