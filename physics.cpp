
#include <iostream>
#include <algorithm>
#include <glm/gtx/projection.hpp>
#include "physics.h"
#include "cityGenerator.h"
#include "graphics.h"

// declared like this so can be accessed from static methods
const int MAX_DYNAMICS = 10000;
std::vector<ColliderData> Physics::dynamicObjects(MAX_DYNAMICS);
std::vector<int> Physics::freeDynamics;
std::vector<AABB> Physics::aabbTree;
std::vector<std::vector<int>> Physics::dynamicMatrix;

Physics::Physics() {
    for (int i = MAX_DYNAMICS; i > 0; --i) {
        freeDynamics.push_back(i - 1);
    }

    generateCollisionMatrix(glm::vec3(0.0f));
}

// if you want to add these back in later for science
// then implement as a static variables in AABB class that is set in calls to AABB::check and sweeptest
//int totalAABBChecks = 0;
//int totalSweepTests = 0;

void Physics::update(float delta) {
    bool printedErrorThisFrame = false;
    int numberOver = 0;

    // clear out all data from supermatrix
    for (size_t i = 0, len = dynamicMatrix.size(); i < len; ++i) {
        dynamicMatrix[i].clear();
    }

    // build leaf list for each valid dynamic and add it to superMatrix
    for (int dndx = 0; dndx < MAX_DYNAMICS; ++dndx) {
        auto& dobj = dynamicObjects[dndx];
        auto& dynamicLeafList = dynamicLeafLists[dndx];
        dynamicLeafList.clear();
        auto& col = dobj.collider;
        // ensure dynamic object is active and awake
        if (dobj.id < 0 || !col.awake) {
            continue;
        }

        // gets list of leafs this object is in (using swept AABB)
        getLeafs(dynamicLeafList, AABB::getSwept(col.getAABB(), col.vel * delta));

        // BASIC can continue from here because they aren't being collided against
        if (col.type == ColliderType::BASIC) {
            continue;
        }

        // add your dynamic objects index into the superMatrix 
        // at every leaf you could possibly be in
        for (size_t i = 0, len = dynamicLeafList.size(); i < len; ++i) {
            dynamicMatrix[dynamicLeafList[i]].push_back(dndx);
        }
    }

    //// add each static object into superMatrix
    //std::vector<int> staticLeafList;
    //for (int ondx = 0, len = static_cast<int>(staticObjects.size()); ondx < len; ++ondx) {
    //    staticLeafList.clear();

    //    // gets list of leafs this object is in 
    //    getLeafs(staticLeafList, staticObjects[ondx]);

    //    // add your static object list index into the super lookup matrix 
    //    // at every leaf youre at
    //    for (size_t i = 0, len = staticLeafList.size(); i < len; ++i) {
    //        superMatrix[staticLeafList[i]].push_back(leafObject{ ondx, false });
    //    }
    //}


    // for each dynamic object check it against all objects in its leaf(s)
    // leafs objects are looked up using the superMatrix
    // leafs can have static and dynamic objects in them
    for (size_t dndx = 0; dndx < MAX_DYNAMICS; ++dndx) {
        auto& dobj = dynamicObjects[dndx];
        Collider& col = dobj.collider;
        if (dobj.id < 0 || !col.awake) {
            col.vel = glm::vec3(0.0f);
            col.grounded = false;
            continue;
        }

        // current dynamic position
        col.pos = Graphics::getTransform(col.transform)->getPos();

        staticResolvedSet.clear();
        dynamicResolvedSet.clear();

        col.vel.y += GRAVITY * col.gravityMultiplier * delta;

        // set remaining velocity to initial velocity of dynamic
        glm::vec3 rvel = col.vel;

        // try to resolve up to 10 collisions for this object this frame
        for (int resolutionAttempts = 0; resolutionAttempts < 10; ++resolutionAttempts) {
            //if (resolutionAttempts == 9 && !printedErrorThisFrame) {
                //std::cout << "PHYSICS::MAX_RESOLUTIONS_REACHED ";
                //numberOver++;
                //printedErrorThisFrame = true;   // to avoid spam
            //}

            // should try only clearing these at beginning of dynamic not each resolution
            staticCheckSet.clear();
            dynamicCheckSet.clear();

            AABB curDynamic = col.getAABB();
            AABB broadphase = AABB::getSwept(curDynamic, rvel * delta);

            // save time, normal, and index of closest object we hit
            float time = 1.0f;  // holds time of collision (0-1)
            glm::vec3 norm;
            int closestIndex = -1;    // index of closest static obj u hit
            bool closestIsDynamic = false;

            // returns closest collision found
            bool fullTest = false;
            // for each leaf this dynamic is in
            std::vector<int>& leafList = dynamicLeafLists[dndx];
            for (size_t lndx = 0, llen = leafList.size(); lndx < llen; ++lndx) {
                int leaf = leafList[lndx];
                // for each object in this leaf
                if (col.type != ColliderType::BASIC) {
                    std::vector<int>& leafObjects = dynamicMatrix[leaf];
                    for (size_t ondx = 0, olen = leafObjects.size(); ondx < olen; ++ondx) {
                        int index = leafObjects[ondx];    // other dynamic object
                        if (dynamicResolvedSet.count(index) || dynamicCheckSet.count(index) ||
                            dndx == index) {
                            continue;
                        }

                        // pretend like they aren't moving since you are going first
                        AABB otherAABB = dynamicObjects[index].collider.getAABB();

                        // broadphase sweep bounds check
                        if (!AABB::check(broadphase, otherAABB)) {
                            // if failed any broadphase then dont check this static again since 
                            // future resolutions will never exceed previous broadphases
                            dynamicResolvedSet.insert(index);
                            continue;
                        }
                        // this set just tracks if youve checked this object before during the current resolution attempt 
                        // done purely to prevent duplicate checks over leaf borders (need to test if this is even worth it lol)
                        dynamicCheckSet.insert(index);

                        // narrow sweep bounds resolution
                        // calculates exact time of collision
                        // but still possible for no collision at this point
                        glm::vec3 n;
                        float t = AABB::sweepTest(curDynamic, otherAABB, rvel * delta, n);
                        if (t < time) { // a collision occured
                            time = t;
                            norm = n;
                            closestIndex = index;
                            closestIsDynamic = true;
                        }
                        fullTest = true;    // this could maybe be in if above?
                    }
                }

                std::vector<int>& sleafObjects = staticMatrix[leaf];
                for (size_t ondx = 0, olen = sleafObjects.size(); ondx < olen; ++ondx) {
                    int index = sleafObjects[ondx];    // other static object
                    if (staticResolvedSet.count(index) || staticCheckSet.count(index)) {
                        continue;
                    }

                    // pretent like they aren't moving since you are going first
                    // cant collide two swept AABBs
                    AABB& otherAABB = staticObjects[index];

                    // broadphase sweep bounds check
                    if (!AABB::check(broadphase, otherAABB)) {
                        // if failed any broadphase then dont check this static again since 
                        // future resolutions will never exceed previous broadphases
                        staticResolvedSet.insert(index);
                        continue;
                    }
                    // this set just tracks if youve checked this object before during the current resolution attempt 
                    // done purely to prevent duplicate checks over leaf borders (need to test if this is even worth it lol)
                    staticCheckSet.insert(index);

                    // narrow sweep bounds resolution
                    // calculates exact time of collision
                    // but still possible for no collision at this point
                    glm::vec3 n;
                    float t = AABB::sweepTest(curDynamic, otherAABB, rvel * delta, n);
                    if (t < time) { // a collision occured
                        time = t;
                        norm = n;
                        closestIndex = index;
                        closestIsDynamic = false;
                    }
                    fullTest = true;    // could be placed in above if statement probably
                }
            }

            if (closestIndex >= 0) {    // collided with another object
                if (closestIsDynamic) {
                    // dont let this dynamic collide with this other object again (this frame)
                    dynamicResolvedSet.insert(closestIndex);

                    ColliderData& other = dynamicObjects[closestIndex];

                    if (dobj.entity != nullptr) {
                        dobj.entity->onCollision(CollisionData{ other.collider.type, other.collider.tag });
                    }
                    if (other.entity != nullptr) {
                        other.entity->onCollision(CollisionData{ col.type, col.tag });
                    }

                    // if your type is TRIGGER or their type is TRIGGER
                    // then reset collision variables to ignore the collision basically
                    if (col.type == ColliderType::TRIGGER ||
                        other.collider.type == ColliderType::TRIGGER) {
                        time = 1.0f;
                        norm = glm::vec3(0.0f);
                    }
                } else {
                    staticResolvedSet.insert(closestIndex);
                }
            }

            col.pos += rvel * delta * time;

            // check height of terrain
            float h = terrainGen->queryHeight(col.pos.x, col.pos.z);

            // ground the object if it hits terrain or normal of what it hits is flat (top of building)
            // check to make sure normal is pointing up actually now
            if (col.pos.y < h || norm.y > 0.0f) {
                col.grounded = true;
                col.vel.y = 0.0f;
                rvel.y = 0.0f;
            }

            if (norm.y < 0.0f) {
                col.vel.y = 0.0f;
                rvel.y = 0.0f;
            }

            // dont let dynamic go below terrain height
            col.pos.y = fmax(col.pos.y, h);

            // if there was a collision then update remaining velocity for subsequent collision tests
            if (time < 1.0f && norm != glm::vec3(0.0f)) {
                // to slide along surface take projection of velocity onto normal of surface
                // and subtract that from current velocity
                glm::vec3 proj = glm::proj(rvel, norm);
                rvel = (rvel - proj) * (1.0f - time);
            } else {
                rvel = glm::vec3(0.0f);
                break;
            }

            // if there was no full collision test then this object is resolved
            // aka no broadphase checks passed so no chance in another collision this frame
            if (!fullTest) {
                break;
            }

        }

        // update transforms position after fully resolved
        Graphics::getTransform(col.transform)->setPos(col.pos);
    }

    //std::cout << numberOver << std::endl;
    //std::cout << "AABB checks: " << totalAABBChecks << " Sweep tests: " << totalSweepTests << std::endl;
    //std::cout << dynamicObjects.size() << std::endl;
    //std::cout << getNumberOfIntersections() << std::endl;

}

int Physics::getNumberOfIntersections() {
    int count = 0;
    for (size_t dndx = 0; dndx < MAX_DYNAMICS; ++dndx) {
        auto& dobj = dynamicObjects[dndx];
        Collider& col = dobj.collider;
        if (dobj.id < 0 || !col.awake) {
            continue;
        }
        AABB mine = col.getAABB();
        std::vector<int>& leafList = dynamicLeafLists[dndx];
        for (size_t lndx = 0, llen = leafList.size(); lndx < llen; ++lndx) {
            int leaf = leafList[lndx];
            // for each object in this leaf
            if (col.type != ColliderType::BASIC) {
                std::vector<int>& leafObjects = dynamicMatrix[leaf];
                for (size_t ondx = 0, olen = leafObjects.size(); ondx < olen; ++ondx) {
                    int index = leafObjects[ondx];    // other dynamic object
                    if (index == dndx) {
                        continue;
                    }

                    AABB theirs = dynamicObjects[index].collider.getAABB();
                    if (AABB::check(mine, theirs)) {
                        count++;
                    }
                }
            }

            std::vector<int>& sleafObjects = staticMatrix[leaf];
            for (size_t ondx = 0, olen = sleafObjects.size(); ondx < olen; ++ondx) {
                int index = sleafObjects[ondx];    // other static object

                if (AABB::check(mine, staticObjects[index])) {
                    count++;
                }

            }
        }
    }

    return count;
}


// should only calculate this if player moves far away enough from it
// like 100.0 units away then recalculate centered on player!!!
// for now just do every 2 seconds or something
void Physics::generateCollisionMatrix(glm::vec3 center) {
    center.y = 0.0f;
    float size = MATRIX_SIZE;
    AABB collisionArea(glm::vec3(-size, 0.0, -size) + center, glm::vec3(size, 10000.0f, size) + center);
    aabbTree.clear();
    aabbTree.push_back(collisionArea);  // start with full collision area
    int startIndex = 0;
    for (int s = 0; s < SPLIT_COUNT; s++) {
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
    // doesn't save that much space and complicates the indexing
    staticMatrix.resize(aabbTree.size());
    dynamicMatrix.resize(aabbTree.size());
    dynamicLeafLists.resize(MAX_DYNAMICS);

    // prints the length of a leafs x/z edge in the tree
    //std::cout << "leaf size: " << size / pow(2, SPLIT_COUNT) << std::endl;
}

void Physics::setCollisionCallback(Entity* entity) {
    dynamicObjects[entity->collider].entity = entity;
}

// add static to statics list and then 
// insert it into supermatrix
void Physics::addStatic(AABB obj) {
    // push new object onto statics list and get index
    staticObjects.push_back(obj);
    int ondx = staticObjects.size() - 1;

    std::vector<int> leafsThisObjectIsIn;
    getLeafs(leafsThisObjectIsIn, obj);

    // add your static object list index into the super lookup matrix 
    // at every leaf youre at
    for (size_t i = 0, len = leafsThisObjectIsIn.size(); i < len; ++i) {
        staticMatrix[leafsThisObjectIsIn[i]].push_back(ondx);
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
        for (size_t j = 0, jlen = staticMatrix[indices[i]].size(); j < jlen; ++j) {
            if (AABB::check(obj, staticObjects[staticMatrix[indices[i]][j]])) {
                return true;
            }
        }
    }
    return false;
}

void Physics::clearStatics() {
    staticObjects.clear();
    staticMatrix.clear();
    staticMatrix.resize(aabbTree.size());
}

int Physics::registerDynamic(int transform) {
    if (freeDynamics.empty()) {
        std::cout << "NO FREE DYNAMIC COLLIDERS";
        return -1;
    }
    int index = freeDynamics.back();
    freeDynamics.pop_back();
    dynamicObjects[index].id = index;
    dynamicObjects[index].collider.transform = transform;
    return index;
}

void Physics::sendOverlapEvent(AABB aabb, CollisionData data) {
    std::vector<int> indices;
    getLeafs(indices, aabb);

    for (size_t i = 0, ilen = indices.size(); i < ilen; ++i) {
        for (size_t j = 0, jlen = dynamicMatrix[indices[i]].size(); j < jlen; ++j) {
            ColliderData& cd = dynamicObjects[dynamicMatrix[indices[i]][j]];
            if (cd.entity != nullptr && AABB::check(aabb, cd.collider.getAABB())) {
                cd.entity->onCollision(data);
            }
        }
    }

}

//void Physics::returnDynamic(int id) {
//}

Collider* Physics::getCollider(int index) {
    assert(index >= 0 && index < MAX_DYNAMICS);
    if (dynamicObjects[index].id < 0) {
        return nullptr;
    }
    return &dynamicObjects[index].collider;
}

int Physics::getColliderModels(std::vector<glm::mat4>& models, std::vector<glm::vec3>& colors) {
    int count = 0;
    int max = models.size();
    for (size_t i = 0, len = staticObjects.size(); i < len; ++i) {
        models[count] = staticObjects[i].getModelMatrix();
        colors[count] = glm::vec3(1.0f, 1.0f, 0.0f);
        if (++count >= max) {
            return count - 1;
        }
    }

    for (size_t i = 0; i < MAX_DYNAMICS; ++i) {
        auto& pobj = dynamicObjects[i];
        if (pobj.id < 0) {
            continue;
        } else if (!pobj.collider.awake) {
            colors[count] = glm::vec3(0.0f, 1.0f, 1.0f);
        } else {
            colors[count] = glm::vec3(1.0f, 0.0f, 0.0f);
        }
        models[count] = pobj.collider.getAABB().getModelMatrix();
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
void Physics::getLeafs(std::vector<int>& locs, AABB aabb) {
    htreesize = static_cast<int>(aabbTree.size());
    htree = &aabbTree;
    hnodes = &locs;
    hswept = aabb;
    checkLeaves(0);
}

