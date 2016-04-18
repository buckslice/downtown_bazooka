#pragma once
#include <glm/glm.hpp>
#include "aabb.h"
#include "transform.h"

enum class ColliderType {
    NOCLIP,         // collides with nothing, movement influenced purely by velocity and gravity
    BASIC,          // collides with statics, invisible to other types
    TRIGGER,        // collides with statics, checks TRIGGERs and FULLs without colliding
    FULL            // collides with statics, checks TRIGGERs and FULLs, collides against FULLs
};

// to be used in collision callbacks
// maybe just make collision callback except a simplified collider struct
// with a enum tag and position and stuff maybe
enum class Tag {
    NONE,
    PLAYER,
    PLAYER_PROJECTILE,
    ENEMY_PROJECTILE,
    ENEMY,
    EXPLOSION,
    TERRAIN, 
	ITEM,
    HEALER
};

// Colliders represent dynamic physics objects
class Collider {
public:
    Transform* transform;  // id of transform this collider is changing position of

    Tag tag;
    ColliderType type;

    glm::vec3 vel;
    bool grounded = false;  // true if is standing on something flat
    bool onTerrain = false; // true if standing on terrain
    bool enabled = true;    // true if should be apart of physics simulation
    bool awake = true;      // true if inside physics simulation area

    float gravityMultiplier = 1.0f;

    Collider();

    void setExtents(glm::vec3 min, glm::vec3 max);

    AABB getAABB();

private:
    glm::vec3 extmin, extmax;

    glm::vec3 pos;
    friend class Physics;
};