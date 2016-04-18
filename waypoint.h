#pragma once

#include "entity.h"

class Waypoint : public Entity {
public:
    Waypoint();
    ~Waypoint();

    void onCollision(Tag tag, Entity* other) override;

    void update(GLfloat delta);

private:
    std::vector<Transform> particleSpawners;

};
