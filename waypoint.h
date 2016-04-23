#pragma once

#include "entity.h"
#include "player.h"

class Waypoint : public Entity {
public:
    Waypoint(Player* p);
    ~Waypoint();

    void onCollision(Tag tag, Entity* other) override;

    void update(GLfloat delta);

    bool resetSpawn = true;

    void reset();

    Player* player;
private:
    std::vector<Transform> particleSpawnPoints;

    AABB currentBuilding;
    glm::vec3 targetPos;
    glm::vec3 lastPos;
    int quadrant = -1;
    int timesTriggered = 0;
    bool triggered = false;
    bool enabled = true;
    float blendTime = 0.0f;

};
