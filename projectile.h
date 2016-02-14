#pragma once
#include "entity.h"
class Projectile : public Entity {
public:
    Projectile();
    ~Projectile();

    void update(GLfloat delta) override;


    void init(int id);

private:
    int id;
    float timer;
};

