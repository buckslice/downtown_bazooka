#include "boss.h"
#include "physics.h"
#include "graphics.h"
#include "entityManager.h"
#include "audio.h"

Boss::Boss(glm::vec3 pos, Player* player) {
    this->player = player;

    body = registerModel();
    larm = registerModel();
    rarm = registerModel();
    leye = registerModel();
    reye = registerModel();
    mouth = registerModel();
    lmouth = registerModel();
    rmouth = registerModel();
    lbrow = registerModel();
    rbrow = registerModel();

    startPosition = pos + glm::vec3(0.0f, 290.0f, 0.0f);
    targetPosition = startPosition;
    transform->setPos(startPosition);
    transform->setVisibility(Visibility::HIDE_SELF);

    body->setScale(50.0f, 50.0f, 50.0f);
    body->color = glm::vec3(0.3f, 0.0f, 0.7f);

    glm::vec3 armScale = glm::vec3(20.0f, 20.0f, 80.0f);
    larm->setPos(-35.0f, 0.0f, 0.0f);
    larm->setScale(armScale);
    larm->color = glm::vec3(1.0f, 0.0f, 1.0f);
    rarm->setPos(35.0f, 0.0f, 0.0f);
    rarm->setScale(armScale);
    rarm->color = glm::vec3(1.0f, 0.0f, 1.0f);

    rgun.setPos(0.0f, 0.0f, 40.0f);
    rarm->parentAll(&rgun);
    lgun.setPos(0.0f, 0.0f, 40.0f);
    larm->parentAll(&lgun);

    leye->setScale(10.0f, 10.0f, 2.0f);
    leye->setPos(-10.0f, 10.0f, 26.0f);
    leye->color = glm::vec3(1.0f, 0.0f, 0.0f);
    reye->setScale(10.0f, 10.0f, 2.0f);
    reye->setPos(10.0f, 10.0f, 26.0f);
    reye->color = glm::vec3(1.0f, 0.0f, 0.0f);

    mouth->setScale(20.0f, 5.0f, 2.0f);
    mouth->setPos(0.0f, -10.0f, 26.0f);
    lmouth->setScale(10.0f, 5.0f, 2.0f);
    lmouth->setPos(-12.0f, -6.0f, 25.5f);
    lmouth->setRot(0.0f, 0.0f, -45.0f);
    rmouth->setScale(10.0f, 5.0f, 2.0f);
    rmouth->setPos(12.0f, -6.0f, 25.5f);
    rmouth->setRot(0.0f, 0.0f, 45.0f);

    lbrow->setScale(15.0f, 5.0f, 4.0f);
    lbrow->setPos(-10.0f, 17.0f, 27.0f);
    lbrow->setRot(0.0f, 0.0f, -30.0f);
    lbrow->color = body->color;
    rbrow->setScale(15.0f, 5.0f, 4.0f);
    rbrow->setPos(10.0f, 17.0f, 27.0f);
    rbrow->setRot(0.0f, 0.0f, 30.0f);
    rbrow->color = body->color;

    transform->parentAll(body);
    body->parentAll(larm, rarm, leye, reye, mouth, lmouth, rmouth, lbrow, rbrow);

    collider->type = ColliderType::FULL;
    collider->tag = Tag::BOSSBOY;
    collider->gravityMultiplier = 0.0f;
    glm::vec3 extents = glm::vec3(25.0f);
    collider->setExtents(-extents, extents);

    Physics::setCollisionCallback(this);
}


Boss::~Boss() {
    std::cout << "BOSS DESTRUCTED" << std::endl;
}

void Boss::update(GLfloat delta) {
    if (health <= 0) {
        bossDyingTime -= delta;
        if (bossDyingTime > 0.0f) {
            for (int i = 0; i < 10; ++i) {
                EntityManagerInstance->SpawnParticle(
                    ParticleType::FIRE, transform->getWorldPos(),
                    Mth::randOnUnitSphere() * 300.0f, 0.0f, glm::vec3(5.0f), false);
            }
        }
        transform->setVisibility(Visibility::HIDE_ALL);
        return;
    }

    glm::vec3 targetDir = glm::normalize(player->transform->getWorldPos() - transform->getWorldPos());
    if (targetDir != glm::vec3(0.0f)) {
        targRot = glm::normalize(glm::rotation(glm::vec3(0.0f, 0.0f, 1.0f), targetDir));
    }
    // slerp keeps the quaternion normalized throughout
    currRot = glm::slerp(currRot, targRot, delta * 2.0f);
    body->setRot(currRot);

    // if vulnerable time just ended then reset pos and spawn new switch
    if (vulnerableTimer > 0.0f && vulnerableTimer - delta < 0.0f) {
        targetPosition = startPosition;
    }
    transform->setPos(Mth::lerp(transform->getPos(), targetPosition, delta));
    vulnerableTimer -= delta;

    float healthPercent = health / maxHealth;
    if (healthPercent > 0.5f) {
        phase = 0;
    } else {
        phase = 1;
    }

    switch (phase) {
    case 0:
        // spawn new switch if none spawned and invulnerable
        if (switchIndex < 0) {
            if (vulnerableTimer < 0.0f) {
                switchIndex = Mth::randRange(0, boxIndices.size());

                Physics::removeStatic(boxIndices[switchIndex]);
                boxIndices[switchIndex] = Physics::addStatic(boxAABBs[switchIndex], Tag::SWITCH);
                oldSwitchColor = boxColors[switchIndex];
                boxColors[switchIndex] = glm::vec3(1.0f, 1.0f, 0.5f);
            }
        } else {    // spawn particles for switch
            AABB box = boxAABBs[switchIndex];

            float x = Mth::randRange(box.min.x, box.max.x);
            float z = Mth::randRange(box.min.z, box.max.z);
            glm::vec3 pos = glm::vec3(x, box.max.y, z);

            for (int i = 0; i < 3; ++i) {
                EntityManagerInstance->SpawnParticle(SWITCH, pos, glm::vec3(), 0.0f, glm::vec3(1.0f), false);
            }
        }

        // take shots at player
        shotTimer -= delta;
        if (shotTimer < 0.0f) {
            shotTimer = 1.0f / shotsPerSecond;
            if (++shotCount > 8) {
                shotTimer += 4.0f;
                shotCount = 0;
            }
            AudioInstance->playSound(Resources::get().bossShoot, 0.25f);

            // get shot position and direction
            glm::vec3 shotPos = shootRight ? rgun.getWorldPos() : lgun.getWorldPos();
            glm::vec3 aimPos = player->transform->getWorldPos() + Mth::randInsideUnitCube() * 20.0f;
            glm::vec3 dir = aimPos - shotPos;
            if (dir != glm::vec3(0.0f)) {
                dir = glm::normalize(dir);
            }
            shootRight = !shootRight;
            EntityManagerInstance->SpawnProjectile(ProjectileType::BOSS_CANNON, Tag::ENEMY_PROJECTILE, shotPos, dir * 100.0f);

        }

        break;
    case 1:
        vulnerableTimer = 10.0f;
        // take shots at player
        shotTimer -= delta;
        if (shotTimer < 0.0f) {
            if (launchHomingVolley) {
                launchHomingVolley = false;
                for (int i = 0; i < 3; ++i) {
                    EntityManagerInstance->SpawnProjectile(ProjectileType::BOSS_HOMING, Tag::ENEMY_PROJECTILE,
                        rgun.getWorldPos(), Mth::randOnUnitSphere() * 100.0f);
                    EntityManagerInstance->SpawnProjectile(ProjectileType::BOSS_HOMING, Tag::ENEMY_PROJECTILE,
                        lgun.getWorldPos(), Mth::randOnUnitSphere() * 100.0f);
                }
                shotTimer = 1.0f;
            } else {

                shotTimer = 0.6f / shotsPerSecond;
                if (++shotCount > 8) {
                    shotTimer = 2.0f;
                    shotCount = 0;
                    launchHomingVolley = true;
                }
                AudioInstance->playSound(Resources::get().bossShoot, 0.3f);

                // get shot position and direction
                glm::vec3 shotPos = shootRight ? rgun.getWorldPos() : lgun.getWorldPos();
                glm::vec3 aimPos = player->transform->getWorldPos();
                glm::vec3 dir = aimPos - shotPos;
                if (dir != glm::vec3(0.0f)) {
                    dir = glm::normalize(dir);
                }
                shootRight = !shootRight;
                EntityManagerInstance->SpawnProjectile(ProjectileType::BOSS_CANNON, Tag::ENEMY_PROJECTILE, shotPos, dir * 100.0f);
            }
        }

        break;
    default:
        break;
    }

}

bool Boss::isVulnerable() {
    return vulnerableTimer > 0.0f;
}
float Boss::getHealthPercentage() {
    return health / maxHealth;
}

void Boss::render() {
    Graphics::addToStream(Shape::CUBE_GRID, boxModels, boxColors);
}

void Boss::playerHitSwitch() {
    targetPosition += glm::vec3(0.0f, -20.0f, 0.0f);

    // restore switch to previous box
    Physics::removeStatic(boxIndices[switchIndex]);
    boxIndices[switchIndex] = Physics::addStatic(boxAABBs[switchIndex]);
    boxColors[switchIndex] = oldSwitchColor;

    // if 3rd switch hit then make boss vulnerable to attack briefly
    if (++switchesHit % 3 == 0) {
        switchIndex = -1;
        vulnerableTimer = 20.0f;
        AudioInstance->playSound(Resources::get().bossVulnerable);
        return; // dont make new switch (do it after vulnerable time ends)
    } else {
        AudioInstance->playSound(Resources::get().bossSwitch);
    }

    // reroll new random index and make switch
    int newIndex = switchIndex;
    while (newIndex == switchIndex) {
        newIndex = Mth::randRange(0, boxIndices.size());
    }
    switchIndex = newIndex;

    Physics::removeStatic(boxIndices[switchIndex]);
    boxIndices[switchIndex] = Physics::addStatic(boxAABBs[switchIndex], Tag::SWITCH);
    oldSwitchColor = boxColors[switchIndex];
    boxColors[switchIndex] = glm::vec3(1.0f, 1.0f, 0.5f);
}


void Boss::onCollision(Tag tag, Entity* other) {
    if (vulnerableTimer < 0.0f) {
        return; // or something
    }
    if (tag == Tag::PLAYER_PROJECTILE || tag == Tag::EXPLOSION) {
        health -= EntityManagerInstance->getPlayerDamage();
    }
}

void Boss::addBox(AABB box, glm::vec3 color) {
    boxIndices.push_back(Physics::addStatic(box));;
    boxAABBs.push_back(box);
    boxModels.push_back(box.getModelMatrix());
    boxColors.push_back(color);
}