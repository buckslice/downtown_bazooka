#pragma once
#include "entity.h"
class Projectile : public Entity {
public:
    Projectile();
    ~Projectile();

    void update(GLfloat delta) override;


    void init(int id, glm::vec3 pos, glm::vec3 vel);

private:
    int id;
    float timer;
};

