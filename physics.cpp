
#include <iostream>
#include <algorithm>
#include <glm/gtx/projection.hpp>
#include "physics.h"
#include "cityGenerator.h"

// has to be in implementation file because reasons (#staticlife?)
//static std::vector<PhysicsTransform> dynamicObjects;

static Pool<PhysicsTransform>* dynamicObjects;

Physics::Physics() {
    dynamicObjects = new Pool<PhysicsTransform>(6000);

    int splits = 6;

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

void Physics::update(float delta) {
    //for (int i = 0; i < dynamicObjects.size(); i++) {
    //    PhysicsTransform& pt = dynamicObjects[i];
    //    if (!pt.alive) {
    //        continue;
    //    }
    //    pt.lpos += pt.vel * delta;
    //}
    //return;

    totalAABBChecks = 0;
    // if in corner between two statics this will be 3 which seems weird, but its because
    // it does 2 initial sweeps, chooses closer collision and resolves it, then it does
    // another sweep from new location against second for a total of 3 sweeps
    int totalSweepTests = 0;

    bool printedErrorThisFrame = false;
    // find list of leaf(s) each dynamic object is in
    dynamicLeafLists.resize(dynamicObjects->size());
    auto& dobjs = dynamicObjects->getObjects();
    for (size_t i = 0, len = dynamicLeafLists.size(); i < len; ++i) {
        dynamicLeafLists[i].clear();
        if (dobjs[i].id < 0) {
            continue;
        }
        getLeafs(dynamicLeafLists[i], dobjs[i].data.getSwept(delta));
    }

    // for each dynamic object
    for (size_t dndx = 0, dlen = dobjs.size(); dndx < dlen; ++dndx) {
        auto& pobj = dobjs[dndx];
        if (pobj.id < 0) {
            continue;
        }
        PhysicsTransform& pt = pobj.data;

        resolvedSet.clear();

        // set remaining velocity to initial velocity of dynamic
        if (pt.getAffectedByGravity()) {
            pt.vel.y += GRAVITY * pt.gravityMultiplier * delta;
        }
        glm::vec3 rvel = pt.vel;

        // try to resolve up to 10 collisions for this object this frame
        for (int resolutionAttempts = 0; resolutionAttempts < 10; ++resolutionAttempts) {
            // probly an error when this happens
            // my aabb sweeptest fix needs a little tweaking
            // could also be something else though perhaps with the quadtree
            // or the different sets pruning too aggresively
            if (resolutionAttempts == 9 && !printedErrorThisFrame) {
                //std::cout << "PHYSICS::MAX_RESOLUTIONS_REACHED ";
                printedErrorThisFrame = true;   // to avoid spam
            }

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
            std::vector<int>& leafList = dynamicLeafLists[dndx];
            for (size_t lndx = 0, llen = leafList.size(); lndx < llen; ++lndx) {
                int leaf = leafList[lndx];
                // for each object in this leaf
                std::vector<int>& leafObjects = treeMatrix[leaf];
                for (size_t ondx = 0, olen = leafObjects.size(); ondx < olen; ++ondx) {
                    int curStaticIndex = leafObjects[ondx];
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
            pt.setPos(pt.getPos() + rvel * delta * time);

            // check height of terrain
            glm::vec3 pos = pt.getPos();
            float h = tg->queryHeight(pos.x, pos.z);

            // ground the object if it hits terrain or normal of what it hits is flat (top of building)
            if (pos.y < h || norm.y != 0.0f) {
                pt.grounded = true;
                pt.vel.y = 0.0f;
                rvel.y = 0.0f;
            }

            // dont let dynamic go below terrain height
            pt.setPos(pos.x, fmax(pos.y, h), pos.z);

            // if there was a collision then update remaining velocity for subsequent collision tests
            if (time < 1.0f) {
                // to slide along surface take projection of velocity onto normal of surface
                // and subtract that from current velocity
                glm::vec3 pvel = rvel - glm::proj(rvel, norm);
                // update remaining velocity to projected velocity * remaining time
                rvel = pvel * (1.0f - time);

                // should add different collision type for reflect bounce too
                //rvel *= time;
                //float eps = 0.0001f;
                //if (abs(norm.x) > eps) {
                //    rvel.x = -rvel.x;
                //    pt.vel.x = -pt.vel.x;
                //}
                //if (abs(norm.y > eps)) {
                //    rvel.y = -rvel.y;
                //    pt.vel.y = -pt.vel.y;
                //}
                //if (abs(norm.z > eps)) {
                //    rvel.z = -rvel.z;
                //    pt.vel.z = -pt.vel.z;
                //}
            }

            // if there was no full collision test then this object is resolved
            if (!fullTest) {
                break;
            }

        }
    }

    //std::cout << "AABB checks: " << totalAABBChecks << " Sweep tests: " << totalSweepTests << std::endl;
    //std::cout << dynamicObjects.size() << std::endl;

}

void Physics::addStatic(AABB obj) {
    // push new object onto statics list and get index
    staticObjects.push_back(obj);
    int objIndex = staticObjects.size() - 1;

    // get list of leaves this object collides with
    std::vector<int> indices;
    getLeafs(indices, obj);
    // then add this objects index to each collision list
    for (size_t i = 0, len = indices.size(); i < len; ++i) {
        treeMatrix[indices[i]].push_back(objIndex);
    }
}

void Physics::addStatics(const std::vector<AABB>& objs) {
    for (size_t i = 0, len = objs.size(); i < len; ++i) {
        addStatic(objs[i]);
    }
}

bool Physics::checkStatic(AABB obj) {
    std::vector<int> indices;
    getLeafs(indices, obj);

    for (size_t i = 0, ilen = indices.size(); i < ilen; ++i) {
        for (size_t j = 0, jlen = treeMatrix[indices[i]].size(); j < jlen; ++j) {
            if (AABB::check(obj, staticObjects[treeMatrix[indices[i]][j]])) {
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
//void Physics::clearDynamics() {
//    PhysicsTransform player = dynamicObjects[0];
//    dynamicObjects.clear();
//    dynamicObjects.push_back(player);
//}

void Physics::printStaticMatrix() {
    for (size_t i = 0, ilen = treeMatrix.size(); i < ilen; ++i) {
        if (treeMatrix[i].size() > 0) {
            std::cout << i << " ";
            for (size_t j = 0, jlen = treeMatrix[i].size(); j < jlen; ++j) {
                std::cout << treeMatrix[i][j] << " ";
            }
            std::cout << std::endl;
        }
    }
}


int Physics::registerDynamic() {
    return dynamicObjects->get();
}

void Physics::returnDynamic(int id) {
    dynamicObjects->ret(id);
}

PhysicsTransform* Physics::getTransform(int index) {
    return dynamicObjects->getData(index);
}

int Physics::getColliderModels(std::vector<glm::mat4>& models, std::vector<glm::vec3>& colors) {
    int count = 0;
    int max = models.size();
    for (size_t i = 0, len = staticObjects.size(); i < len; ++i) {
        glm::vec3 pos = staticObjects[i].getCenter();
        glm::vec3 scale = staticObjects[i].getSize();
        glm::mat4 model;
        model = glm::translate(model, pos);
        model = glm::scale(model, scale);
        models[count] = model;
        colors[count] = glm::vec3(1.0f, 1.0f, 0.0f);
        if (++count >= max) {
            return count - 1;
        }
    }

    auto& dobjs = dynamicObjects->getObjects();
    for (size_t i = 0, len = dynamicObjects->size(); i < len; ++i) {
        auto& pobj = dobjs[i];
        //if (pobj.id < 0) {
        //    continue;
        //}

        if (pobj.id < 0) {
            colors[count] = glm::vec3(0.0f, 1.0f, 1.0f);
        } else {
            colors[count] = glm::vec3(1.0f, 0.0f, 0.0f);
        }
        AABB b = pobj.data.getAABB();
        glm::vec3 pos = b.getCenter();
        glm::vec3 scale = b.getSize();
        glm::mat4 model;
        model = glm::translate(model, pos);
        model = glm::scale(model, scale);
        models[count] = model;
        if (++count >= max) {
            return count - 1;
        }
    }

    return count;
}


// this should never be called directly so its hidden down here super sketchily
// so things cant see it, except the one function that calls it below #prostrats 
// should probably just make another class lol
// also took all the variables out of function call for #maxperfomance
int htreesize = 0;
std::vector<AABB> *htree;
std::vector<int>* hnodes;
AABB hswept;
void checkLeaves(int node) {
    totalAABBChecks++;
    if (AABB::check((*htree)[node], hswept)) {
        // check if you are leaf node
        if (node * 4 + 1 >= htreesize) {
            hnodes->push_back(node);
        } else {
            // exlore children
            checkLeaves(node * 4 + 1);
            checkLeaves(node * 4 + 2);
            checkLeaves(node * 4 + 3);
            checkLeaves(node * 4 + 4);
        }
    }
}

// searches tree and returns a list of leaf indices AABB collides with
void Physics::getLeafs(std::vector<int>& locs, AABB swept) {
    htreesize = static_cast<int>(aabbTree.size());
    htree = &aabbTree;
    hnodes = &locs;
    hswept = swept;
    checkLeaves(0);
}

