
#include <iostream>
#include <algorithm>
#include <glm/gtx/projection.hpp>
#include "physics.h"
#include "cityGenerator.h"

// has to be in implementation file because reasons (#staticlife?)
// should probably research alternatives to this cuz its gross lol
static Pool<Collider>* dynamicObjects;

Physics::Physics() {
    dynamicObjects = new Pool<Collider>(10000);

    generateCollisionMatrix(glm::vec3(0.0f));
}

// if you want to add these back in later for science
// then implement as a static variables in AABB class that is set in calls to AABB::check and sweeptest
//int totalAABBChecks = 0;
//int totalSweepTests = 0;

void Physics::update(float delta) {
    bool printedErrorThisFrame = false;

    auto& dobjs = dynamicObjects->getObjects();
    const size_t sizeOfDynamicObjects = dobjs.size();
    dynamicLeafLists.resize(sizeOfDynamicObjects);

    // clear out all data from supermatrix
    for (size_t i = 0, len = superMatrix.size(); i < len; ++i) {
        superMatrix[i].clear();
    }

    // build leaf list for each valid dynamic and add it to superMatrix
    for (size_t dndx = 0; dndx < sizeOfDynamicObjects; ++dndx) {
        auto& dobj = dobjs[dndx];
        auto& dynamicLeafList = dynamicLeafLists[dndx];
        dynamicLeafList.clear();
        // ensure dynamic object is active and awake
        if (dobj.id < 0 || !dobj.data.awake) {
            continue;
        }

        // gets list of leafs this object could possibly be in (using swept AABB)
        getLeafs(dynamicLeafList, dobj.data.getSwept(delta));

        // BASIC can continue from here because they aren't being collided against
        if (dobj.data.type == ColliderType::BASIC) {
            continue;
        }

        // add your dynamic objects index into the superMatrix 
        // at every leaf you could possibly be in
        for (size_t i = 0, len = dynamicLeafList.size(); i < len; ++i) {
            superMatrix[dynamicLeafList[i]].push_back(leafObject{ dndx, false });
        }
    }

    // add each static object into superMatrix
    std::vector<size_t> leafsThisObjectIsIn;
    for (size_t objIndex = 0; objIndex < staticObjects.size(); ++objIndex) {
        leafsThisObjectIsIn.clear();

        // gets list of leafs this object is in 
        getLeafs(leafsThisObjectIsIn, staticObjects[objIndex]);

        // add your static object list index into the super lookup matrix 
        // at every leaf youre at
        for (size_t i = 0, len = leafsThisObjectIsIn.size(); i < len; ++i) {
            superMatrix[leafsThisObjectIsIn[i]].push_back(leafObject{ objIndex, false });
        }
    }


    // for each dynamic object check it against all objects in its leaf(s)
    // leafs objects are looked up using the superMatrix
    // leafs can have static and dynamic objects in them
    for (size_t dndx = 0; dndx < sizeOfDynamicObjects; ++dndx) {
        auto& pobj = dobjs[dndx];
        Collider& col = pobj.data;
        if (pobj.id < 0 || !col.awake) {
            continue;
        }
        col.pos = Graphics::getTransform(col.transform)->getPos();

        staticResolvedSet.clear();
        dynamicResolvedSet.clear();

        // set remaining velocity to initial velocity of dynamic
        col.vel.y += GRAVITY * col.gravityMultiplier * delta;

        glm::vec3 rvel = col.vel;

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

            staticCheckSet.clear();
            dynamicCheckSet.clear();

            AABB curDynamic = col.getAABB();
            AABB broadphase = AABB::getSwept(curDynamic, col.vel * delta);

            // save time, normal, and index of closest object we hit
            float time = 1.0f;  // holds time of collision (0-1)
            glm::vec3 norm;
            int closestIndex = -1;    // index of closest static obj u hit
            bool closestIsDynamic = false;

            // returns closest collision found
            bool fullTest = false;
            // for each leaf this dynamic is in
            std::vector<size_t>& leafList = dynamicLeafLists[dndx];
            for (size_t lndx = 0, llen = leafList.size(); lndx < llen; ++lndx) {
                int leaf = leafList[lndx];
                // for each object in this leaf
                std::vector<leafObject>& leafObjects = superMatrix[leaf];
                for (size_t ondx = 0, olen = leafObjects.size(); ondx < olen; ++ondx) {
                    leafObject& obj = leafObjects[ondx];    // single other static or dynamic object
                    AABB otherAABB;
                    if (obj.dynamic) {
                        if (dynamicResolvedSet.count(obj.index) || dynamicCheckSet.count(obj.index)) {
                            continue;
                        }

                        // pretent like they aren't moving since you are going first
                        // cant collide two swept AABBs
                        otherAABB = dobjs[obj.index].data.getAABB();

                    } else {
                        if (staticResolvedSet.count(obj.index) || staticCheckSet.count(obj.index)) {
                            continue;
                        }

                        otherAABB = staticObjects[obj.index];
                    }

                    // broadphase sweep bounds check
                    if (!AABB::check(broadphase, otherAABB)) {
                        // if failed any broadphase then dont check this static again since 
                        // future resolutions will never exceed previous broadphases
                        if (obj.dynamic) {
                            dynamicResolvedSet.insert(obj.index);
                        } else {
                            staticResolvedSet.insert(obj.index);
                        }
                        continue;
                    }
                    // this set just tracks if youve checked this object before during the current resolution attempt 
                    // done purely to prevent duplicate checks over leaf borders (need to test if this is even worth it lol)
                    if (obj.dynamic) {
                        dynamicCheckSet.insert(obj.index);
                    } else {
                        staticCheckSet.insert(obj.index);
                    }

                    // narrow sweep bounds resolution
                    // calculates exact time of collision
                    // but still possible for no collision at this point
                    glm::vec3 n;
                    float t = AABB::sweepTest(curDynamic, otherAABB, rvel * delta, n);
                    if (t < time) {
                        time = t;
                        norm = n;
                        closestIndex = obj.index;
                        closestIsDynamic = obj.dynamic;
                    }
                    fullTest = true;
                }
            }

            // dont let this dynamic collide with this other object again (this frame)
            if (closestIsDynamic) {
                dynamicResolvedSet.insert(closestIndex);
            } else {
                staticResolvedSet.insert(closestIndex);
            }

            // update dynamic position
            col.pos += rvel * delta * time;

            // check height of terrain
            float h = tg->queryHeight(col.pos.x, col.pos.z);

            // ground the object if it hits terrain or normal of what it hits is flat (top of building)
            if (col.pos.y < h || norm.y != 0.0f) {
                col.grounded = true;
                col.vel.y = 0.0f;
                rvel.y = 0.0f;
            }

            // dont let dynamic go below terrain height
            col.pos.y = fmax(col.pos.y, h);

            // if there was a collision then update remaining velocity for subsequent collision tests
            if (time < 1.0f) {
                // to slide along surface take projection of velocity onto normal of surface
                // and subtract that from current velocity
                glm::vec3 pvel = rvel - glm::proj(rvel, norm);
                // update remaining velocity to projected velocity * remaining time
                rvel = pvel * (1.0f - time);

                // should add different collision type for reflect bounce too
                // initial tests arent working yet though lol...
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

        // updates graphics position after fully resolved
        Graphics::getTransform(col.transform)->setPos(col.pos);
    }

    //std::cout << "AABB checks: " << totalAABBChecks << " Sweep tests: " << totalSweepTests << std::endl;
    //std::cout << dynamicObjects.size() << std::endl;

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
    superMatrix.resize(aabbTree.size());

    // prints the length of a leafs x/z edge in the tree
    //std::cout << "leaf size: " << size / pow(2, SPLIT_COUNT) << std::endl;
}


void Physics::addStatic(AABB obj) {
    // push new object onto statics list and get index
    staticObjects.push_back(obj);
    int objIndex = staticObjects.size() - 1;

    //// get list of leaves this object collides with
    //std::vector<int> indices;
    //getLeafs(indices, obj);
    //// then add this objects index to each collision list
    //for (size_t i = 0, len = indices.size(); i < len; ++i) {
    //    treeMatrix[indices[i]].push_back(objIndex);
    //}
}

void Physics::addStatics(const std::vector<AABB>& objs) {
    for (size_t i = 0, len = objs.size(); i < len; ++i) {
        addStatic(objs[i]);
    }
}

bool Physics::checkStatic(AABB obj) {
    std::vector<size_t> indices;
    getLeafs(indices, obj);

    for (size_t i = 0, ilen = indices.size(); i < ilen; ++i) {
        for (size_t j = 0, jlen = superMatrix[indices[i]].size(); j < jlen; ++j) {
            leafObject& lo = superMatrix[indices[i]][j];
            if (lo.dynamic) {   // skip dynamic objects in this leaf
                continue;
            }
            if (AABB::check(obj, staticObjects[lo.index])) {
                return true;
            }
        }
    }
    return false;
}

//void Physics::clearStatics() {
//    staticObjects.clear();
//    treeMatrix.clear();
//    treeMatrix.resize(aabbTree.size());
//}

int Physics::registerDynamic(int transform) {
    int index = dynamicObjects->get();
    getCollider(index)->transform = transform;
    return index;
}

//void Physics::returnDynamic(int id) {
//    dynamicObjects->ret(id);
//}

Collider* Physics::getCollider(int index) {
    return dynamicObjects->getData(index);
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

    auto& dobjs = dynamicObjects->getObjects();
    for (size_t i = 0, len = dynamicObjects->getSize(); i < len; ++i) {
        auto& pobj = dobjs[i];

        if (pobj.id < 0) {
            continue;
        } else if (!pobj.data.awake) {
            colors[count] = glm::vec3(0.0f, 1.0f, 1.0f);
        } else {
            colors[count] = glm::vec3(1.0f, 0.0f, 0.0f);
        }
        models[count] = pobj.data.getAABB().getModelMatrix();
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
size_t htreesize = 0;
std::vector<AABB> *htree;
std::vector<size_t>* hnodes;
AABB hswept;
void checkLeaves(size_t node) {
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
void Physics::getLeafs(std::vector<size_t>& locs, AABB swept) {
    htreesize = aabbTree.size();
    htree = &aabbTree;
    hnodes = &locs;
    hswept = swept;
    checkLeaves(0);
}

