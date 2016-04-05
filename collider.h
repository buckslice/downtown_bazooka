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
    DEFAULT,
    PLAYER,
    PLAYER_PROJECTILE,
    ENEMY_PROJECTILE,
    ENEMY,
    EXPLOSION,
    TERRAIN, 
	ITEM_HEAL,
	ITEM_STAMINA,
	ITEM_STRENGTH,
	ITEM_AGILITY,
	ITEM_DEXTERITY
};

struct CollisionData {
    ColliderType type;
    ColliderTag tag;
};

// Colliders represent dynamic physics objects
class Collider {
public:
    int transform;  // id of transform this collider is changing position of

    ColliderTag tag;
    ColliderType type;

    glm::vec3 vel;
    bool grounded = false, enabled = true, awake = true;

    float gravityMultiplier = 1.0f;

    Collider();

    void setExtents(glm::vec3 min, glm::vec3 max);

    AABB getAABB();

private:
    glm::vec3 extmin, extmax;

    glm::vec3 pos;
    friend class Physics;
};