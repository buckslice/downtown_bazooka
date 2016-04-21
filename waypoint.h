#pragma once

#include "entity.h"
#include "player.h"

class Waypoint : public Entity {
public:
    Waypoint();
    ~Waypoint();

    void onCollision(Tag tag, Entity* other) override;

    void update(GLfloat delta);

    bool firstSpawn = true;
    Player* player;
private:
    std::vector<Transform> particleSpawnPoints;

    glm::vec3 targetPos;
    glm::vec3 lastPos;
    int quadrant = -1;
    int successes = 0;
    bool active = false;
    float blendTime = 0.0f;

};
