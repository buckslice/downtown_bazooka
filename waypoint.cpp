#include "waypoint.h"
#include "entityManager.h"

Waypoint::Waypoint() {
    transform->setPos(glm::vec3(300.0f, 50.0f, 0.0f));
    collider->type = ColliderType::TRIGGER;
    collider->gravityMultiplier = 0.0f;
    collider->setExtents(glm::vec3(-2.0f, 0.0f, -2.0f), glm::vec3(2.0f, 4.0f, 2.0f));
    //transform->setVisibility = Visibility::HIDDEN;

    float r = 10.0f;
    Transform t1(glm::vec3(r, 0.0f, 0.0f));
    t1.setVisibility(Visibility::HIDE_SELF);
    Transform t2(glm::vec3(0.0f, 0.0f, r));
    t2.setVisibility(Visibility::HIDE_SELF);
    Transform t3(glm::vec3(-r, 0.0f, 0.0f));
    t3.setVisibility(Visibility::HIDE_SELF);
    Transform t4(glm::vec3(0.0f, 0.0f, -r));
    t4.setVisibility(Visibility::HIDE_SELF);

    transform->parentAll(&t1, &t2, &t3, &t4);

    particleSpawners.push_back(t1);
    particleSpawners.push_back(t2);
    particleSpawners.push_back(t3);
    particleSpawners.push_back(t4);
}

Waypoint::~Waypoint() {

}

void Waypoint::update(GLfloat delta) {
    transform->rotate(delta*360.0f, glm::vec3(0.0f, 1.0f, 0.0f));
    for (size_t i = 0; i < particleSpawners.size(); ++i) {
        glm::vec3 pos = particleSpawners[i].getWorldPos();
        EntityManagerInstance->SpawnParticle(ParticleType::BEACON, pos, glm::vec3(0.0f, 100.0f, 0.0f), 1.0f, glm::vec3(5.0f), false);
    }
}


void Waypoint::onCollision(Tag tag, Entity* other) {
    if (tag == Tag::PLAYER) {
        // turn and fly away somewhere lol
    }

}