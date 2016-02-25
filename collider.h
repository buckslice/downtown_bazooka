#pragma once
#include <glm/glm.hpp>
#include "aabb.h"

enum ColliderType {
    BASIC,          // collides with statics, invisible to other types
    TRIGGER,        // collides with statics, checks TRIGGERs and FULLs without colliding
    FULL            // collides with statics, checks TRIGGERs and FULLs, collides against FULLs
};

// to be used in collision callbacks
// maybe just make collision callback except a simplified collider struct
// with a enum tag and position and stuff maybe
enum ColliderTag {
    PLAYER,
    PLAYER_PROJECTILE,
    ENEMY_PROJECTILE,
    ENEMY,
    TERRAIN, 
    DEFAULT
};

// Collider is a class for moving physics objects
class Collider {
public:
    int transform;  // id of transform this collider is changing position of

    ColliderTag tag;
    ColliderType type;

    glm::vec3 vel;
    bool grounded = false;
    bool awake = true;

    float gravityMultiplier = 1.0f;

    Collider();

    void setExtents(glm::vec3 min, glm::vec3 max);

    AABB getAABB();
    AABB getSwept(float delta);    // uses vel

    // function to set when you want to be notified of who you collided with
    void(*onCollisionCallback)(Collider* other);
    // could maybe make it fire an event in future event system too perhaps?

private:
    glm::vec3 extmin;   // (-0.5f, 0.0f, -0.5f)
    glm::vec3 extmax;   // (0.5f, 2.0f, 0.5f)

    // gets set at beginning of each physics iteration
    // could prob make this better somehow (two out of cache jumps per collider per frame (probly? (i have no clue))
    glm::vec3 pos; 
    friend class Physics;   // so physics can directly set pos but nobody else
};