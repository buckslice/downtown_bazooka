
#include <iostream>
#include <algorithm>
#include <glm/gtx/projection.hpp>
#include "physics.h"
#include "graphics.h"
#include "game.h"


Quadtree* Physics::collisionTree;

static MemPool<ColliderData> dynamicPool(15000);
static MemPool<StaticData> staticPool(15000);

std::vector<OverlapEvent> Physics::overlapEvents;

Physics::Physics() {
    collisionTree = new Quadtree(0, getPhysicsArea(glm::vec3(0.0f), MATRIX_SIZE));
}

Physics::~Physics() {
    delete collisionTree;
}

void Physics::rebuildCollisionTree(float delta) {
    delete collisionTree;
    collisionTree = new Quadtree(0, getPhysicsArea(Game::getPlayerPos(), MATRIX_SIZE));
    
    // insert all statics into tree
    for (StaticData* sd = nullptr; staticPool.next(sd);) {
        collisionTree->insert(QuadtreeData{ sd->bounds, staticPool.getIndex(sd), false });
    }
    if (delta == 0.0f) {
        return;
    }
    // insert all dynamics into tree (that are awake and not basic)
    for (ColliderData* cd = nullptr; dynamicPool.next(cd);) {
        Collider& col = cd->collider;
        // ensure object is active
        if (!col.enabled || col.type == ColliderType::BASIC || col.type == ColliderType::NOCLIP) {
            continue;
        }
        // update collider position from transform
        col.pos = col.transform->getPos();

        col.awake = collisionTree->insert(
            QuadtreeData{ AABB::getSwept(col.getAABB(), col.vel*delta), dynamicPool.getIndex(cd), true });

    }
}

void Physics::processOverlapEvents(std::vector<QuadtreeData>& returnData) {
    // process and send out all overlap events since last frame
    for (size_t i = 0; i < overlapEvents.size(); ++i) {
        OverlapEvent& oe = overlapEvents[i];

        collisionTree->retrieve(returnData, oe.bounds);

        for (size_t i = 0, len = returnData.size(); i < len; ++i) {
            QuadtreeData& qtd = returnData[i];
            if (qtd.dynamic) {
                ColliderData* cd = dynamicPool.get(qtd.index);
                if (cd->entity && AABB::check(oe.bounds, cd->collider.getAABB())) {
                    cd->entity->onCollision(oe.tag, oe.entity);
                }
            }
        }
    }
    overlapEvents.clear(); // clear event list for next frame
}

void Physics::update(float delta) {

    rebuildCollisionTree(delta);

    // used by rest of update method 
    std::vector<QuadtreeData> returnData;

    processOverlapEvents(returnData);

    // for each dynamic object check it against all objects in its leaf(s)
    // leafs objects are looked up using the superMatrix
    // leafs can have static and dynamic objects in them
    for (ColliderData* cdObj = nullptr; dynamicPool.next(cdObj);) {
        Collider& col = cdObj->collider;
        col.onTerrain = col.grounded = false;
        if (!col.enabled || !col.awake) {
            col.vel = glm::vec3(0.0f);
            continue;
        }
        if (col.type == ColliderType::NOCLIP) {
            col.vel.y += GRAVITY * col.gravityMultiplier * delta;
            col.transform->addPos(col.vel * delta);
            continue;
        }

        // make sure to do this again incase new objects were created since built tree up above
        col.pos = col.transform->getPos();

        staticResolvedSet.clear();
        dynamicResolvedSet.clear();

        col.vel.y += GRAVITY * col.gravityMultiplier * delta;

        returnData.clear();
        collisionTree->retrieve(returnData, AABB::getSwept(col.getAABB(), col.vel * delta));

        // set remaining velocity to initial velocity of dynamic
        glm::vec3 rvel = col.vel;

        int dndx = dynamicPool.getIndex(cdObj);
        // try to resolve up to 10 collisions for this object this frame
        for (int resolutionAttempts = 0; resolutionAttempts < 10; ++resolutionAttempts) {

            // should try only clearing these at beginning of dynamic not each resolution
            staticCheckSet.clear();
            dynamicCheckSet.clear();

            AABB myAABB = col.getAABB();
            AABB broadphase = AABB::getSwept(myAABB, rvel * delta);

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
                    if (col.type == ColliderType::BASIC || dndx == index ||
                        dynamicResolvedSet.count(index) || dynamicCheckSet.count(index)) {
                        continue;
                    }

                    ColliderData* other = dynamicPool.get(index);
                    // pretend like they aren't moving since you are going first
                    AABB otherAABB = other->collider.getAABB();

                    // check if already intersecting and if so just call onCollision and continue
                    if (AABB::check(myAABB, otherAABB)) {
                        // dont let this dynamic collide with this other object again (this frame)
                        dynamicResolvedSet.insert(index);

                        if (cdObj->entity) {
                            cdObj->entity->onCollision(other->collider.tag, other->entity);
                        }
                        if (other->entity) {
                            other->entity->onCollision(col.tag, cdObj->entity);
                        }
                        // actually dont continue because dynamics touching eachother will
                        // count as colliding here barely
                        //continue;
                    }

                    // broadphase sweep bounds check to see if these two objects will collide this frame
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
                    float t = AABB::sweepTest(myAABB, otherAABB, rvel * delta, n);
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
                    AABB& otherAABB = qtd.box;

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
                    float t = AABB::sweepTest(myAABB, otherAABB, rvel * delta, n);
                    if (t < time) { // a collision occured
                        time = t;
                        norm = n;
                        closestIndex = index;
                        closestIsDynamic = false;
                    }
                    fullTest = true;    // could be placed in above if statement probably
                }
            }

            if (closestIndex >= 0 && closestIsDynamic) {
                // if your type is TRIGGER or their type is TRIGGER
                // then reset collision variables to pretent like it didn't happen
                if (col.type == ColliderType::TRIGGER ||
                    dynamicPool.get(closestIndex)->collider.type == ColliderType::TRIGGER) {
                    time = 1.0f;
                    norm = glm::vec3(0.0f);
                }
            }

            // update position
            col.pos += rvel * delta * time;

            // check height of terrain
            float h = terrainGen->queryHeight(col.pos.x, col.pos.z);
            if (h < -100.0f) {  // no terrain loaded currently at this x,z position
                col.awake = false;
                break;
            }

            // ground the object if it hits terrain or normal of what it hits is flat (top of building)
            // check to make sure normal is pointing up actually now
            if (col.pos.y < h) {
                col.onTerrain = true;
                col.grounded = true;
                col.vel.y = 0.0f;
                rvel.y = 0.0f;
            } else if (norm.y != 0.0f) {
                col.grounded = true;
                col.vel.y = 0.0f;
                rvel.y = 0.0f;
            }

            // dont let dynamic go below terrain height
            col.pos.y = fmax(col.pos.y, h);

            if (closestIndex >= 0) {    // collided with another object
                if (closestIsDynamic) {
                    // dont let this dynamic collide with this other object again (this frame)
                    dynamicResolvedSet.insert(closestIndex);

                    ColliderData* other = dynamicPool.get(closestIndex);
                    if (cdObj->entity) {
                        cdObj->entity->onCollision(other->collider.tag, other->entity);
                    }
                    if (other->entity) {
                        other->entity->onCollision(col.tag, cdObj->entity);
                    }
                } else {
                    // get tag from static and send to entity
                    StaticData* other = staticPool.get(closestIndex);
                    if (other->tag != Tag::NONE && cdObj->entity) {
                        cdObj->entity->onCollision(other->tag);
                    }

                    staticResolvedSet.insert(closestIndex);
                }
            }

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

        }  // end of resolutions loop for current dynamic

        // update transforms position after fully resolved
        col.transform->setPos(col.pos);
    }

    //std::cout << numberOver << std::endl;
    //std::cout << "AABB checks: " << totalAABBChecks << " Sweep tests: " << totalSweepTests << std::endl;
    //std::cout << dynamicObjects.size() << std::endl;
}

void Physics::setCollisionCallback(Entity* entity) {
    int index = dynamicPool.getIndex((char*)entity->collider);
    dynamicPool.get(index)->entity = entity;
}

// add aabb to static object list
int Physics::addStatic(AABB bounds, Tag tag) {
    StaticData* sd = staticPool.alloc();
    if (!sd) {
        std::cout << "NO FREE STATIC COLLIDERS" << std::endl;
        return -1;
    }
    sd->bounds = bounds;
    sd->tag = tag;
    int index = staticPool.getIndex(sd);
    // inserts right away so can checkStatic without having
    // to wait until next frame for tree to be built
    collisionTree->insert(QuadtreeData{ sd->bounds, index, false });
    return index;
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

void Physics::removeStatic(int index) {
    if (index < 0) {
        return;
    }
    staticPool.free(index);
}

Collider* Physics::registerDynamic(Transform* transform) {
    assert(transform && "Physics::registerDynamic");

    ColliderData* cd = dynamicPool.alloc();
    if (!cd) {
        std::cout << "NO FREE DYNAMIC COLLIDERS";
        return nullptr;
    }
    cd->collider.transform = transform;
    return &cd->collider;
}

void Physics::returnDynamic(Collider* collider) {
    int index = dynamicPool.getIndex((char*)collider);
    dynamicPool.free(index);
}

AABB Physics::getPhysicsArea(glm::vec3 center, float size) {
    center.y = 0.0f;
    return AABB(glm::vec3(-size, -10.0, -size) + center, glm::vec3(size, 10000.0f, size) + center);
}

void Physics::sendOverlapEvent(AABB bounds, Tag tag, Entity* entity) {
    overlapEvents.push_back(OverlapEvent{ bounds, tag, entity });
}

void Physics::streamColliderModels() {
    for (StaticData* sd = nullptr; staticPool.next(sd);) {
        Graphics::addToStream(Shape::CUBE_SOLID, sd->bounds.getModelMatrix(), glm::vec3(1.0f, 1.0f, 0.0f));
    }

    for (ColliderData* cd = nullptr; dynamicPool.next(cd);) {
        glm::vec3 color;
        Collider& col = cd->collider;

        // never draw NOCLIP colliders because they only use their collider
        // for simple velocity and gravity updates from physics engine
        if (col.type == ColliderType::NOCLIP) {
            continue;
        }

        if (!col.enabled) {
            // ignore drawing deactivated particles to avoid cluttering
            if (col.type == ColliderType::BASIC) {
                continue;
            }
            // this pretty much never happens now since
            // since everything is pooled now and its memory is 
            // corrupted as soon as its returned
            color = glm::vec3(0.7f);  // grey for disabled colliders
        } else if (!col.awake) {
            color = glm::vec3(0.0f, 1.0f, 1.0f);    // teal for sleeping colliders
        } else {
            color = glm::vec3(1.0f, 0.0f, 0.0f);    // red for active dynamic colliders
        }

        Graphics::addToStream(Shape::CUBE_SOLID, col.getAABB().getModelMatrix(), color);
    }

    // white for quadtree
    collisionTree->streamModel(glm::vec3(1.0f));

}

// finds point on top of tallest building in certain quadrant 
// in relation to origin point
// quadrant layout
// 0 -> +x -z
// 1 -> -x -z
// 2 -> -x +z
// 3 -> +x +z
AABB Physics::getTallestBuildingInQuadrant(glm::vec3 origin, int quadrant) {
    AABB tallest;
    for (StaticData* sd = nullptr; staticPool.next(sd);) {
        AABB& b = sd->bounds;
        glm::vec3 c = b.getCenter();
        // continue if building not in correct quadrant
        switch (quadrant) {
        case 0:
            if (c.x < origin.x || c.z > origin.z) {
                continue;
            } break;
        case 1:
            if (c.x > origin.x || c.z > origin.z) {
                continue;
            } break;
        case 2:
            if (c.x > origin.x || c.z < origin.z) {
                continue;
            } break;
        case 3:
            if (c.x < origin.x || c.z < origin.z) {
                continue;
            } break;
        default:
            std::cout << "BAD QUADRANT" << std::endl;
            return AABB();
            break;
        }
        // continue if building is too close to current origin
        float minDist = 400.0f;
        glm::vec3 d = c - origin;
        if (glm::dot(d, d) < minDist * minDist) {
            continue;
        }
        // continue if building is too small (like a tree)
        if (b.getSize().x < 5.0f || b.getSize().z < 5.0f) {
            continue;
        }

        if (b.max.y > tallest.max.y) {
            tallest = b;
        }
    }
    return tallest;
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

// QUADTREE LAYOUT
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

void Quadtree::streamModel(glm::vec3 color) {
    if (hasChildren) {
        for (int i = 0; i < 4; ++i) {
            nodes[i]->streamModel(color);
        }
    }
    AABB mbounds = bounds;
    mbounds.min.y = 0.0f;
    mbounds.max.y = 0.1f;
    Graphics::addToStream(Shape::CUBE_SOLID, mbounds.getModelMatrix(), color);
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