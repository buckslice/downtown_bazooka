#pragma once

#include "entity.h"
#include "player.h"

class Waypoint : public Entity {
public:
    Waypoint(Player* p);
    ~Waypoint();

    void onCollision(Tag tag, Entity* other) override;

    void update(GLfloat delta);


    void reset();

    Player* player;
private:
    std::vector<Transform> particleSpawnPoints;

    AABB currentBuilding;
    glm::vec3 targetPos;
    glm::vec3 lastPos;
    float blendTime = 0.0f;
    int timesTriggered = 0;
    int quadrant = -1;
    bool resetSpawn = true;
    bool triggered = false;
    bool enabled = true;

};
