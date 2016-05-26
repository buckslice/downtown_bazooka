#include "waypoint.h"
#include "entityManager.h"
#include "terrain.h"
#include "physics.h"
#include "audio.h"
#include "game.h"

Waypoint::Waypoint(Player* p) {
    this->player = p;
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

void Waypoint::reset() {
    this->enabled = !Game::isInFinalBattle();
    collider->enabled = !Game::isInFinalBattle();
    timesTriggered = 0; // reset times triggered
    resetSpawn = true;
}

void Waypoint::update(GLfloat delta) {
    if (!enabled) {
        return;
    }

    // if havent spawned into world yet then do so
    if (resetSpawn) {
        if (!Terrain::hardGenerating) {
            // get tallest building in random quadrant and set your pos to top of it
            quadrant = Mth::randRange(0, 4);
            currentBuilding = Physics::getTallestBuildingInQuadrant(Game::getPlayerPos(), quadrant);
            glm::vec3 v = currentBuilding.getCenter();
            v.y = currentBuilding.max.y;
            transform->setPos(v);

            // save last position for lerping between
            lastPos = targetPos = v;

            resetSpawn = false;
        } else {    // if terrain isnt done generating then return
            return;
        }
    }

    // play humming sound if player is close enough and not triggered
    glm::vec3 diff = player->transform->getWorldPos();
    diff -= transform->getWorldPos();
    float range = 200.0f;
    float t = glm::dot(diff, diff) / (range*range);
    if (t < 1.0f && !triggered) {
        AudioInstance->playSoundSingle(Resources::get().waypointSound, (1.0f - t));
    }

    // blend to new target trigger point
    blendTime += delta;
    float b = Mth::cblend(blendTime, 0.0f, 5.0f, Mth::smootherStep);
    transform->setPos(Mth::lerp(lastPos, targetPos, b));
    diff = transform->getPos() - targetPos;
    if (glm::dot(diff, diff) < 1.0f) {
        triggered = false;
    }

    // rotate transform and draw particles
    transform->rotate(delta*360.0f, glm::vec3(0.0f, 1.0f, 0.0f));
    for (size_t i = 0; i < particleSpawnPoints.size(); ++i) {
        glm::vec3 pos = particleSpawnPoints[i].getWorldPos();
        ParticleType t = triggered ? ParticleType::BEACON_TRIGGERED : ParticleType::BEACON;
        EntityManagerInstance->SpawnParticle(t, pos, glm::vec3(0.0f, 100.0f, 0.0f), 1.0f, glm::vec3(5.0f), false);
    }
}

void Waypoint::onCollision(Tag tag, Entity* other) {
    if (!triggered && tag == Tag::PLAYER) {
        glm::vec3 v = player->collider->vel;
        if (!player->collider->grounded || glm::dot(v, v) > 10.0f * 10.0f) {
            return;
        }

        player->addHealth(1000.0f);   // heal player to full

        // spawn boss battle area if third waypoint hit
        if (++timesTriggered >= 0) {
            AudioInstance->playSound(Resources::get().timetodieSound);
            Game::startFinalBattle(currentBuilding);
            Game::setRequiresWorldRegen(true);
            reset();
            return;
        }

        AudioInstance->playSound(Resources::get().waypointHitSound);
        lastPos = transform->getPos();
        currentBuilding = Physics::getTallestBuildingInQuadrant(lastPos, quadrant);
        glm::vec3 c = currentBuilding.getCenter();
        c.y = currentBuilding.max.y;
        targetPos = c;
        blendTime = 0.0f;
        triggered = true;
    }

}