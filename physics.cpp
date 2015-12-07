
#include <iostream>
#include <algorithm>
#include <glm/gtx/projection.hpp>
#include "physics.h"
#include "cityGenerator.h"

// has to be in implementation file because reasons (#staticlife?)
static std::vector<PhysicsTransform> dynamicObjects;

Physics::Physics() {

    int splits = 5;

    // aabb encapsulate whole city
    // should make aabb encapsulate method and change to actually check against all static
    // would then need to rebuild whole tree sometimes tho i think..?
    float hs = CITY_SIZE / 2.0f;
    float wgl = 100.0f;
    AABB wholeCity(glm::vec3(-hs - wgl, 0.0, -hs - wgl), glm::vec3(hs + wgl, 10000.0f, hs + wgl));
    aabbTree.push_back(wholeCity);
    int startIndex = 0;
    for (int s = 0; s < splits; s++) {
        int len = aabbTree.size();
        for (int i = startIndex; i < len; i++) {
            AABB cur = aabbTree[i];

            float hx = cur.min.x + (cur.max.x - cur.min.x) / 2.0f;
            float hz = cur.min.z + (cur.max.z - cur.min.z) / 2.0f;

            // bl tl tr br
            aabbTree.push_back(AABB(cur.min, glm::vec3(hx, cur.max.y, hz)));
            aabbTree.push_back(AABB(glm::vec3(cur.min.x, cur.min.y, hz), glm::vec3(hx, cur.max.y, cur.max.z)));
            aabbTree.push_back(AABB(glm::vec3(hx, cur.min.y, hz), cur.max));
            aabbTree.push_back(AABB(glm::vec3(hx, cur.min.y, cur.min.z), glm::vec3(cur.max.x, cur.max.y, hz)));

        }

        // increment next start index based on how much we added this split
        startIndex += (int)pow(4, s);
    }

    // stores lists of static objects
    // its same size as aabbTree even though it should only just be leaves but whatever
    treeMatrix.resize(aabbTree.size());

    //float citysize = wholeCity.max.x - wholeCity.min.x;
    //std::cout << "leaf size: " << citysize / pow(2, splits) << std::endl;

}

int totalAABBChecks = 0;

// searches tree and returns a list of leaf indices AABB collides with
// externally you always call it with node = 0 so you start at root node
// but wasnt sure how to enforce that
void Physics::getLeafs(std::vector<int>& locs, int node, AABB swept) {
    totalAABBChecks++;
    if (AABB::check(aabbTree[node], swept)) {
        // check if you are leaf node
        if (node * 4 + 1 >= aabbTree.size()) {
            locs.push_back(node);
        } else {
            // exlore children
            getLeafs(locs, node * 4 + 1, swept);
            getLeafs(locs, node * 4 + 2, swept);
            getLeafs(locs, node * 4 + 3, swept);
            getLeafs(locs, node * 4 + 4, swept);
        }
    }
}

void Physics::update(float delta) {

    totalAABBChecks = 0;
    // if in corner between two statics this will be 3 which seems weird, but its because
    // it does 2 initial sweeps, chooses closer collision and resolves it, then it does
    // another sweep from new location against second for a total of 3 sweeps
    int totalSweepTests = 0;

    bool printedErrorThisFrame = false;
    // find list of leaf(s) each dynamic object is in
    dynamicLeafLists.resize(dynamicObjects.size());
    for (int i = 0; i < dynamicLeafLists.size(); i++) {
        dynamicLeafLists[i].clear();
        if (!dynamicObjects[i].alive) {
            continue;
        }
        getLeafs(dynamicLeafLists[i], 0, dynamicObjects[i].getSwept(delta));
    }

    // for each dynamic object
    for (int dynamicIndex = 0; dynamicIndex < dynamicObjects.size(); dynamicIndex++) {
        PhysicsTransform& pt = dynamicObjects[dynamicIndex];

        if (!pt.alive) {
            continue;
        }

        resolvedSet.clear();

        // set remaining velocity to initial velocity of dynamic
        if (pt.getAffectedByGravity()) {
            pt.vel.y += GRAVITY * pt.gravityMultiplier * delta;
        }
        glm::vec3 rvel = pt.vel;

        // try to resolve up to 10 collisions for this object this frame
        for (int resolutionAttempts = 0; resolutionAttempts < 10; resolutionAttempts++) {
            // probly an error when this happens
            // my aabb sweeptest fix needs a little tweaking
            // could also be something else though perhaps with the quadtree
            // or the different sets pruning too aggresively
            //if (resolutionAttempts == 9 && !printedErrorThisFrame) {
            //    std::cout << "PHYSICS::MAX_RESOLUTIONS_REACHED ";
            //    printedErrorThisFrame = true;   // to avoid spam
            //}

            checkSet.clear();

            AABB curDynamic = pt.getAABB();
            AABB broadphase = AABB::getSwept(curDynamic, pt.vel * delta);

            // save time, normal, and index of closest object we hit
            float time = 1.0f;  // holds time of collision (0-1)
            glm::vec3 norm;
            int closestStaticIndex = -1;    // index of closest static obj u hit

            // returns closest collision found
            bool fullTest = false;
            // for each leaf this dynamic is in
            std::vector<int>& leafList = dynamicLeafLists[dynamicIndex];
            for (int li = 0; li < leafList.size(); li++) {
                int leaf = leafList[li];
                // for each object in this leaf
                std::vector<int>& leafObjects = treeMatrix[leaf];
                for (int i = 0; i < leafObjects.size(); i++) {
                    int curStaticIndex = leafObjects[i];
                    if (resolvedSet.count(curStaticIndex) || checkSet.count(curStaticIndex)) {
                        continue;
                    }

                    AABB curStatic = staticObjects[curStaticIndex];
                    totalAABBChecks++;
                    // broadphase sweep bounds check
                    if (!AABB::check(broadphase, curStatic)) {
                        // if failed any broadphase then dont check this static again since 
                        // future resolutions will never exceed previous broadphases
                        resolvedSet.insert(curStaticIndex);
                        continue;
                    }
                    // this set just tracks if youve checked this object before during the current resolution attempt 
                    // done purely to prevent duplicate checks over leaf borders (need to test if this is even worth it lol)
                    checkSet.insert(curStaticIndex);

                    // narrow sweep bounds resolution
                    // calculates exact time of collision
                    // but still possible for no collision at this point
                    glm::vec3 n;
                    float t = AABB::sweepTest(curDynamic, curStatic, rvel * delta, n);
                    if (t < time) {
                        time = t;
                        norm = n;
                        closestStaticIndex = curStaticIndex;
                    }
                    totalSweepTests++;
                    fullTest = true;
                }
            }

            // dont let this dynamic collide with this static again
            resolvedSet.insert(closestStaticIndex);

            // update dynamic position
            pt.pos += rvel * delta * time;

            // ground dynamic if hit bottom or if normal of what you hit points in the y direction
            // should technically only set grounded if normal is > 0.0f actually..
            // also just made all PhysicsTransforms have a grounded variable cuz might be useful, and im lazy
            if (pt.pos.y < 0.0f || norm.y != 0.0f) {
                pt.grounded = true;
                pt.vel.y = 0.0f;
                rvel.y = 0.0f;
            }

            // dont let dynamic go below 0.0f;
            pt.pos.y = fmax(pt.pos.y, 0.0f);

            // if there was a collision then update remaining velocity for subsequent collision tests
            if (time < 1.0f) {
                // to slide along surface take projection of velocity onto normal of surface
                // and subtract that from current velocity
                glm::vec3 pvel = rvel - glm::proj(rvel, norm);
                // update remaining velocity to projected velocity * remaining time
                rvel = pvel * (1.0f - time);
                // should add different collision type for reflect bounce too
            }

            // if there was no full collision test then this object is resolved
            if (!fullTest) {
                break;
            }

        }
    }

    //std::cout << "AABB checks: " << totalAABBChecks << " Sweep tests: " << totalSweepTests << std::endl;

}

void Physics::addStatic(AABB obj) {
    // push new object onto statics list and get index
    staticObjects.push_back(obj);
    int objIndex = staticObjects.size() - 1;

    // get list of leaves this object collides with
    std::vector<int> indices;
    getLeafs(indices, 0, obj);
    // then add this objects index to each collision list
    for (int i = 0; i < indices.size(); i++) {
        treeMatrix[indices[i]].push_back(objIndex);
    }
}

void Physics::addStatics(const std::vector<AABB>& objs) {
    for (int i = 0; i < objs.size(); i++) {
        addStatic(objs[i]);
    }
}

bool Physics::checkStatic(AABB obj) {
    std::vector<int> indices;
    getLeafs(indices, 0, obj);

    for (int i = 0; i < indices.size(); i++) {
        for (int j = 0; j < treeMatrix[indices[i]].size(); j++) {
            int o = treeMatrix[indices[i]][j];
            if (AABB::check(obj, staticObjects[o])) {
                return true;
            }
        }
    }
    return false;
}

void Physics::clearStatics() {
    staticObjects.clear();
    treeMatrix.clear();
    treeMatrix.resize(aabbTree.size());
}

// pretty filth temp method for now until i figure out wth im doing (more on this below lol)
void Physics::clearDynamics() {
    PhysicsTransform player = dynamicObjects[0];
    dynamicObjects.clear();
    dynamicObjects.push_back(player);
}

void Physics::printStaticMatrix() {
    for (int i = 0; i < treeMatrix.size(); i++) {
        if (treeMatrix[i].size() > 0) {
            std::cout << i << " ";
            for (int j = 0; j < treeMatrix[i].size(); j++) {
                std::cout << treeMatrix[i][j] << " ";
            }
            std::cout << std::endl;
        }
    }
}

// need to make this return an int pointer or something
// so it can be reassigned when a dynamic is destroyed or something
// alternatively just store everything in entity and quit caring about cache cuz
// probably doesnt even matter! ya since entity wont even have that many components in it
// just not sure how to handle pointers to different objects basically
// indices into vector works but then need to be updated when vector deletes (just move end guy into deleted spot)
// or with pointers but whenever a vector resizes any pointers to any of its objects get invalidated
int Physics::registerDynamic(glm::vec3 scale) {
    dynamicObjects.push_back(PhysicsTransform(scale));
    return dynamicObjects.size() - 1;
}

//void releaseDynamic

PhysicsTransform* Physics::getTransform(int index) {
    return &dynamicObjects[index];
}

