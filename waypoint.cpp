#include "waypoint.h"
#include "entityManager.h"
#include "terrain.h"
#include "physics.h"
#include "audio.h"

Waypoint::Waypoint() {
    transform->setPos(glm::vec3(0.0f, 3000.0f, 0.0f));
    collider->type = ColliderType::TRIGGER;
    collider->gravityMultiplier = 0.0f;
    collider->setExtents(glm::vec3(-8.0f, 0.0f, -8.0f), glm::vec3(8.0f));
    transform->setVisibility(Visibility::HIDE_ALL);

    float r = 10.0f;
    Transform t1(glm::vec3(r, 0.0f, 0.0f));
    Transform t2(glm::vec3(0.0f, 0.0f, r));
    Transform t3(glm::vec3(-r, 0.0f, 0.0f));
    Transform t4(glm::vec3(0.0f, 0.0f, -r));

    transform->parentAll(&t1, &t2, &t3, &t4);

    particleSpawnPoints.push_back(t1);
    particleSpawnPoints.push_back(t2);
    particleSpawnPoints.push_back(t3);
    particleSpawnPoints.push_back(t4);

    Physics::setCollisionCallback(this);
}

Waypoint::~Waypoint() {
}

void Waypoint::update(GLfloat delta) {
    if (firstSpawn) {
        if (!Terrain::hardGenerating) {
            quadrant = Mth::randRange(0, 4);
            glm::vec3 v = Physics::getTallestBuildingInQuadrant(glm::vec3(0.0f), quadrant);
            transform->setPos(v);
            lastPos = targetPos = v;
            firstSpawn = false;
        } else {
            return;
        }
    }


    glm::vec3 diff = player->transform->getWorldPos();
    diff -= transform->getWorldPos();
    float range = 200.0f;
    float t = glm::dot(diff, diff) / (range*range);
    if (t < 1.0f && active) {
        AudioInstance->playSoundSingle(Resources::get().waypointSound, (1.0f - t));
    }


    blendTime += delta;
    float b = Mth::cblend(blendTime, 0.0f, 5.0f, Mth::smootherStep);

    transform->setPos(Mth::lerp(lastPos, targetPos, b));
    diff = transform->getPos() - targetPos;
    if (glm::dot(diff, diff) < 1.0f) {
        active = true;
    }


    transform->rotate(delta*360.0f, glm::vec3(0.0f, 1.0f, 0.0f));
    for (size_t i = 0; i < particleSpawnPoints.size(); ++i) {
        glm::vec3 pos = particleSpawnPoints[i].getWorldPos();
        ParticleType t = active ? ParticleType::BEACON : ParticleType::BEACON_TRIGGERED;
        EntityManagerInstance->SpawnParticle(t, pos, glm::vec3(0.0f, 100.0f, 0.0f), 1.0f, glm::vec3(5.0f), false);
    }
}

void Waypoint::onCollision(Tag tag, Entity* other) {
    if (active && tag == Tag::PLAYER) {
        AudioInstance->playSound(Resources::get().waypointHitSound);
        player->addHealth(1000.0f);   // heal player to full
        lastPos = transform->getPos();
        glm::vec3 v = Physics::getTallestBuildingInQuadrant(lastPos, quadrant);
        targetPos = v;
        blendTime = 0.0f;
        active = false;
        successes++;
    }

}