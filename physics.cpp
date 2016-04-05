
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
Quadtree* Physics::collisionTree;

Physics::Physics() {
    for (int i = MAX_DYNAMICS; i > 0; --i) {
        freeDynamics.push_back(i - 1);
    }
    collisionTree = new Quadtree(0, getPhysicsArea(glm::vec3(0.0f), MATRIX_SIZE));
}

// if you want to add these back in later for science
// then implement as a static variables in AABB class that is set in calls to AABB::check and sweeptest
//int totalAABBChecks = 0;
//int totalSweepTests = 0;

void Physics::update(float delta, glm::vec3 center) {
    bool printedErrorThisFrame = false;
    int numberOver = 0;

    delete collisionTree;
    collisionTree = new Quadtree(0, getPhysicsArea(center, MATRIX_SIZE));

    // insert all statics into tree
    for (int sndx = 0, len = static_cast<int>(staticObjects.size()); sndx < len; ++sndx) {
        collisionTree->insert(QuadtreeData{ staticObjects[sndx], sndx, false });
    }
    // insert all dynamics into tree (that are awake and not basic)
    for (int dndx = 0; dndx < MAX_DYNAMICS; ++dndx) {
        auto& dobj = dynamicObjects[dndx];
        auto& col = dobj.collider;
        // ensure dynamic object is active and awake
        // if type is basic then dont insert into tree, just retrieve
        if (dobj.id < 0 || !col.enabled || col.type == ColliderType::BASIC) {
            continue;
        }
        col.awake = collisionTree->insert(QuadtreeData{ AABB::getSwept(col.getAABB(), col.vel*delta), dndx, true });
    }

    std::vector<QuadtreeData> returnData;
    // for each dynamic object check it against all objects in its leaf(s)
    // leafs objects are looked up using the superMatrix
    // leafs can have static and dynamic objects in them
    for (size_t dndx = 0; dndx < MAX_DYNAMICS; ++dndx) {
        auto& dobj = dynamicObjects[dndx];
        Collider& col = dobj.collider;
        if (dobj.id < 0 || !col.enabled || !col.awake) {
            col.vel = glm::vec3(0.0f);
            col.grounded = false;
            continue;
        }

        // current dynamic position
        col.pos = Graphics::getTransform(col.transform)->getPos();

        staticResolvedSet.clear();
        dynamicResolvedSet.clear();

        col.vel.y += GRAVITY * col.gravityMultiplier * delta;

        returnData.clear();
        collisionTree->retrieve(returnData, AABB::getSwept(col.getAABB(), col.vel * delta));

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

            // for each object this dynamic could collide against
            for (size_t indx = 0, len = returnData.size(); indx < len; ++indx) {
                QuadtreeData& qtd = returnData[indx];
                int index = qtd.index;
                if (qtd.dynamic) {
                    if (col.type == ColliderType::BASIC ||
                        dynamicResolvedSet.count(index) || dynamicCheckSet.count(index) ||
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
                    fullTest = true;    // could be placed in above if statement probably?

                } else {    // static
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
}

void Physics::setCollisionCallback(Entity* entity) {
    dynamicObjects[entity->collider].entity = entity;
}

// add aabb to static object list
void Physics::addStatic(AABB obj) {
    // push new object onto statics list and get index
    staticObjects.push_back(obj);
    int ondx = staticObjects.size() - 1;
    // inserts right away so can checkStatic without having
    // to wait until next frame for tree to be built
    collisionTree->insert(QuadtreeData{ obj, ondx, false });
}

void Physics::addStatics(const std::vector<AABB>& objs) {
    for (size_t i = 0, len = objs.size(); i < len; ++i) {
        addStatic(objs[i]);
    }
}

// checks static against other statics in the matrix
// returns true if collides with any
bool Physics::checkStatic(AABB obj) {
    std::vector<QuadtreeData> returnData;
    collisionTree->retrieve(returnData, obj);
    for (size_t i = 0, len = returnData.size(); i < len; ++i) {
        if (!returnData[i].dynamic) {
            if (AABB::check(obj, returnData[i].box)) {
                return true;
            }
        }
    }
    return false;
}

void Physics::clearStatics() {
    staticObjects.clear();
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

AABB Physics::getPhysicsArea(glm::vec3 center, float size) {
    center.y = 0.0f;
    return AABB(glm::vec3(-size, -10.0, -size) + center, glm::vec3(size, 1000.0f, size) + center);
}

void Physics::sendOverlapEvent(AABB aabb, CollisionData data) {
    std::vector<QuadtreeData> returnData;
    collisionTree->retrieve(returnData, aabb);

    for (size_t i = 0, len = returnData.size(); i < len; ++i) {
        QuadtreeData& qtd = returnData[i];
        if (qtd.dynamic) {
            ColliderData& cd = dynamicObjects[qtd.index];
            if (cd.entity != nullptr && AABB::check(aabb, cd.collider.getAABB())) {
                cd.entity->onCollision(data);
            }
        }
    }
}

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
        ++count;
    }

    for (size_t i = 0; i < MAX_DYNAMICS; ++i) {
        auto& pobj = dynamicObjects[i];
        if (pobj.id < 0) {
            continue;
        } else if (!pobj.collider.enabled) {
            colors[count] = glm::vec3(0.7f);
        } else if (!pobj.collider.awake) {
            colors[count] = glm::vec3(0.0f, 1.0f, 1.0f);
        } else {
            colors[count] = glm::vec3(1.0f, 0.0f, 0.0f);
        }
        models[count] = pobj.collider.getAABB().getModelMatrix();
        ++count;
    }

    collisionTree->getModel(count, models, colors);

    return count;
}

Quadtree::Quadtree(int level, AABB bounds) {
    this->level = level;
    this->bounds = bounds;
}

Quadtree::~Quadtree() {
    data.clear();
    if (hasChildren) {
        for (int i = 0; i < 4; i++) {
            delete nodes[i];
        }
    }
}

//---------> X axis
//| ---------
//| | 0 | 1 |
//| ---------
//| | 3 | 2 |
//| ---------
//V Z axis

void Quadtree::split() {
    // find x and z at mid point of bounds aabb
    float mx = bounds.min.x + (bounds.max.x - bounds.min.x) / 2.0f;
    float mz = bounds.min.z + (bounds.max.z - bounds.min.z) / 2.0f;

    // tl tr bl br
    nodes[0] = new Quadtree(level + 1,
        AABB(bounds.min, glm::vec3(mx, bounds.max.y, mz)));
    nodes[1] = new Quadtree(level + 1,
        AABB(glm::vec3(mx, bounds.min.y, bounds.min.z), glm::vec3(bounds.max.x, bounds.max.y, mz)));
    nodes[2] = new Quadtree(level + 1,
        AABB(glm::vec3(mx, bounds.min.y, mz), bounds.max));
    nodes[3] = new Quadtree(level + 1,
        AABB(glm::vec3(bounds.min.x, bounds.min.y, mz), glm::vec3(mx, bounds.max.y, bounds.max.z)));

    hasChildren = true;
}

bool Quadtree::insert(QuadtreeData qtd) {
    if (!AABB::check(bounds, qtd.box)) {
        return false;
    }
    insertRecursive(qtd);
    return true;
}

void Quadtree::insertRecursive(QuadtreeData qtd) {
    if (!hasChildren) {
        // if no children then insert into this leaf
        data.push_back(qtd);

        // split this node if has too many objects
        if (static_cast<int>(data.size()) > MAX_OBJECTS && level < MAX_LEVELS) {
            split();

            while (data.size() > 0) {
                insertRecursive(data.back());
                data.pop_back();
            }
        }
        return;
    }

    AABB& b = qtd.box;
    // find x and z at mid point of bounds aabb
    float mx = bounds.min.x + (bounds.max.x - bounds.min.x) / 2.0f;
    float mz = bounds.min.z + (bounds.max.z - bounds.min.z) / 2.0f;
    if (b.min.z < mz) {
        if (b.min.x < mx) {
            nodes[0]->insertRecursive(qtd);
        }
        if (b.max.x >= mx) {
            nodes[1]->insertRecursive(qtd);
        }
    }
    if (b.max.z >= mz) {
        if (b.min.x < mx) {
            nodes[3]->insertRecursive(qtd);
        }
        if (b.max.x >= mx) {
            nodes[2]->insertRecursive(qtd);
        }
    }
}

void Quadtree::retrieve(std::vector<QuadtreeData>& returnData, AABB b) {
    if (!hasChildren) {
        // add this nodes data to end of return data
        returnData.insert(returnData.end(), data.begin(), data.end());
        return;
    }
    // find x and z at mid point of bounds aabb
    float mx = bounds.min.x + (bounds.max.x - bounds.min.x) / 2.0f;
    float mz = bounds.min.z + (bounds.max.z - bounds.min.z) / 2.0f;
    if (b.min.z < mz) {
        if (b.min.x < mx) {
            nodes[0]->retrieve(returnData, b);
        }
        if (b.max.x >= mx) {
            nodes[1]->retrieve(returnData, b);
        }
    }
    if (b.max.z >= mz) {
        if (b.min.x < mx) {
            nodes[3]->retrieve(returnData, b);
        }
        if (b.max.x >= mx) {
            nodes[2]->retrieve(returnData, b);
        }
    }
}

void Quadtree::getModel(int& count, std::vector<glm::mat4>& models, std::vector<glm::vec3>& colors) {
    if (hasChildren) {
        for (int i = 0; i < 4; ++i) {
            nodes[i]->getModel(count, models, colors);
        }
    }
    AABB mbounds = bounds;
    mbounds.min.y = 0.0f;
    mbounds.max.y = 0.1f;
    models[count] = mbounds.getModelMatrix();
    colors[count] = glm::vec3(0.0f, 0.0f, 1.0f);
    ++count;
}

int Quadtree::getNodeCount() {
    int c = 1;
    if (hasChildren) {
        for (int i = 0; i < 4; ++i) {
            c += nodes[i]->getNodeCount();
        }
    }
    return c;
}