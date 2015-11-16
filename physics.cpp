
#include "physics.h"
#include <iostream>
#include <algorithm>
#include <glm/gtx/projection.hpp>

Physics::Physics(Camera& cam) : cam(cam) {
}

// make entity class with pos, vel, and maybe onCollisoin method or something
// make physics loop through each dynamic object and check agaisnt static objects
// the square distance broadphase check will probably be bad in this scenario even 
// need some sort of spatial partitioning :)))) so fun

// renderingengine class
// input class <--- really would help (just pressed tracking)
// keyboard class and mouse class rather
// game class
// ui class?

// oh also circle ground mesh and square groudn mesh that are black colored

void Physics::update(float delta) {

    // for loop will go here for each dynamic object

    resolvedSet.clear();
    // set remaining velocity to camera velocity initially
    glm::vec3 rvel = cam.vel;

    // try to resolve up to 10 collisions for this object this frame
    for (int resolutionAttempts = 0; resolutionAttempts < 10; resolutionAttempts++) {
        // build collision box from player
        glm::vec3 p = cam.pos;
        AABB player(glm::vec3(p.x - .5f, p.y - EYE_HEIGHT, p.z - .5f),
            glm::vec3(p.x + .5f, p.y + (P_HEIGHT - EYE_HEIGHT), p.z + .5f));

        // get box that covers players current position projected by velocity
        AABB broadphase = AABB::getSwept(player, rvel * delta);

        // save time, normal, and index of closest object we hit
        float time = 1.0f;  // holds time of collision (0-1)
        glm::vec3 norm;
        int objIndex = -1;

        // returns closest collision found
        bool fullTest = false;
        for (int i = 0; i < staticObjects.size(); i++) {
            // if already resolved check
            if (resolvedSet.count(i)) {
                continue;
            }

            // square distance check
            glm::vec3 diff = player.min - staticObjects[i].min;
            if (glm::dot(diff, diff) > COL_RADIUS * COL_RADIUS) {
                continue;
            }

            // broadphase sweep bounds check
            if (!AABB::check(broadphase, staticObjects[i])) {
                continue;
            }

            // narrow sweep bounds resolution
            // calculates exact time of collision (if there was one)
            glm::vec3 n;
            float t = sweepTest(player, staticObjects[i], rvel * delta, n);
            if (t < time) {
                time = t;
                norm = n;
                objIndex = i;
            }
            fullTest = true;
        }
        resolvedSet.insert(objIndex);

        // update camera position
        cam.pos += rvel * delta * time;

        // ground camera if hit bottom or if normal of what you hit points in the y direction
        // should technically only set grounded if normal is > 0.0f but whatever
        if (cam.pos.y < EYE_HEIGHT || norm.y != 0.0f) {
            cam.grounded = true;
            cam.vel.y = 0.0f;
            rvel.y = 0.0f;
        }

        // dont let camera go below bottom of level (y = 0 + eye height)
        cam.pos.y = fmax(cam.pos.y, EYE_HEIGHT);

        // if there was a collision then update remaining velocity for subsequent collision tests
        if (time < 1.0f) {
            // to slide along surface take projection of velocity onto normal of surface
            // and subtract that from current velocity
            glm::vec3 pvel = rvel - glm::proj(rvel, norm);
            // update remaining velocity to projected velocity * remaining time
            rvel = pvel * (1.0f - time);
        }

        // if there was no full collision test then this object is resolved
        if (!fullTest) {
            break;
        }

    }

    //std::cout << resolvedSet.size() << std::endl;
}

// TODO put this as a static function in AABB instead
float Physics::sweepTest(AABB b1, AABB b2, glm::vec3 vel, glm::vec3& norm) {
    // find distance between objects on near and far sides
    glm::vec3 invEntr;
    glm::vec3 invExit;

    if (vel.x > 0.0f) {
        invEntr.x = b2.min.x - b1.max.x;
        invExit.x = b2.max.x - b1.min.x;
    } else {
        invEntr.x = b2.max.x - b1.min.x;
        invExit.x = b2.min.x - b1.max.x;
    }
    if (vel.y > 0.0f) {
        invEntr.y = b2.min.y - b1.max.y;
        invExit.y = b2.max.y - b1.min.y;
    } else {
        invEntr.y = b2.max.y - b1.min.y;
        invExit.y = b2.min.y - b1.max.y;
    }
    if (vel.z > 0.0f) {
        invEntr.z = b2.min.z - b1.max.z;
        invExit.z = b2.max.z - b1.min.z;
    } else {
        invEntr.z = b2.max.z - b1.min.z;
        invExit.z = b2.min.z - b1.max.z;
    }

    float e = .00001f;
    if (invEntr.y < e && invEntr.y > -e) {
        invEntr.y = 0.0f;
    }
    // above is my wierd fix for falling through tops of buildings randomly
    // basically its accounting for floating point precision error during earlier subtractions
    // doesnt happen in x or z direction because player movement is much
    // more granular than gravity. prob should just do the next two checks as well
    // but i want to wait to see if a bug pops up and if these will fix it to confirm my theories

    //if (invEntr.x < e && invEntr.x > -e) {
    //    invEntr.x = 0.0f;
    //}
    //if (invEntr.z < e && invEntr.z > -e) {
    //    invEntr.z = 0.0f;
    //}

    // find time of collision
    glm::vec3 entr;
    glm::vec3 exit;

    if (vel.x == 0.0f) {
        entr.x = -std::numeric_limits<float>::infinity();
        exit.x = std::numeric_limits<float>::infinity();
    } else {
        entr.x = invEntr.x / vel.x;
        exit.x = invExit.x / vel.x;
    }
    if (vel.y == 0.0f) {
        entr.y = -std::numeric_limits<float>::infinity();
        exit.y = std::numeric_limits<float>::infinity();
    } else {
        entr.y = invEntr.y / vel.y;
        exit.y = invExit.y / vel.y;
    }
    if (vel.z == 0.0f) {
        entr.z = -std::numeric_limits<float>::infinity();
        exit.z = std::numeric_limits<float>::infinity();
    } else {
        entr.z = invEntr.z / vel.z;
        exit.z = invExit.z / vel.z;
    }

    float entrTime = glm::compMax(entr);
    float exitTime = glm::compMin(exit);

    // if there was no collision
    if (entrTime > exitTime ||
        entr.x < 0.0f && entr.y < 0.0f && entr.z < 0.0f ||
        entr.x > 1.0f || entr.y > 1.0f || entr.z > 1.0f) {
        norm = glm::vec3(0.0f);
        return 1.0f;
    }

    // calculate normal of collided surface
    if (entrTime == entr.x) {
        norm = glm::vec3(invEntr.x < 0.0f ? 1.0f : -1.0f, 0.0f, 0.0f);
    } else if (entrTime == entr.y) {
        norm = glm::vec3(0.0f, invEntr.y < 0.0f ? 1.0f : -1.0f, 0.0f);
    } else {
        norm = glm::vec3(0.0f, 0.0f, invEntr.z < 0.0f ? 1.0f : -1.0f);
    }

    return entrTime;
}

void Physics::addObject(AABB& obj) {
    staticObjects.push_back(obj);
}

void Physics::addObjects(const std::vector<AABB>& objs) {
    for (int i = 0; i < objs.size(); i++) {
        staticObjects.push_back(objs[i]);
    }
}

void Physics::clearObjects() {
    staticObjects.clear();
}