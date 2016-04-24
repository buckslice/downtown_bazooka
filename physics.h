#pragma once

#include <cmath>
#include <vector>
#include <unordered_set>

#include "transform.h"
#include "collider.h"
#include "terrain.h"
#include "entity.h"

struct ColliderData {
    Collider collider;
    Entity* entity = nullptr;
};

struct StaticData {
    AABB bounds;
    Tag tag;
};

struct OverlapEvent {
    AABB bounds;
    Tag tag;
    Entity* entity;
};

struct QuadtreeData {
    AABB box;   // local copy of aabb to reduce index lookups
    int index;
    bool dynamic;
};

class Quadtree;
class Physics {
public:
    const float GRAVITY = -30.0f;

    Physics();
    ~Physics();

    // progress the physics simulation by delta time
    // center dictates the center of the collision area
    void update(float delta);

    // clears tree and reinserts all current statics and dynamics
    void rebuildCollisionTree(float delta);

    // checks all overlap events for this frame
    void processOverlapEvents(std::vector<QuadtreeData>& returnData);

    // adds a static to the matrix (returns index)
    static int addStatic(AABB bounds, Tag tag = Tag::NONE);

    // checks to see if static collides with any in the matrix
    static bool checkStatic(AABB obj);
    static void removeStatic(int index);

    // sets Entity callback pointer for when collision happens
    static void setCollisionCallback(Entity* entity);

    // registers a collider that will control the movement of
    // the given transform and returns that colliders index
    static Collider* registerDynamic(Transform* transform);
    static void returnDynamic(Collider* collider);

    static void sendOverlapEvent(AABB bounds, Tag tag, Entity* entity = nullptr);

    void streamColliderModels();

    static AABB getTallestBuildingInQuadrant(glm::vec3 origin, int quadrant);

    Terrain* terrainGen;   // ref to terrain for collision detection

private:
    const float MATRIX_SIZE = 1500.0f;

    // used for spacial partitioning to reduce physics checks
    // turns an N^2 problem into NlogN
    static Quadtree* collisionTree;

    // stores overlapEvents and executes them after a new tree is built
    static std::vector<OverlapEvent> overlapEvents;

    // returns top level aabb that indicates the boundaries of the physics simulation
    // starts as root node for the collisionTree
    AABB getPhysicsArea(glm::vec3 center, float size);

    // could probably combine these two sets into one 
    // by offsetting static indices by # of dynamics or something
    // used to prevent unnecessary additional checks
    std::unordered_set<int> staticResolvedSet;
    std::unordered_set<int> dynamicResolvedSet;

    // used to prevent multiple checks on leaf borders
    std::unordered_set<int> staticCheckSet;
    std::unordered_set<int> dynamicCheckSet;

};

class Quadtree {
public:
    const int MAX_OBJECTS = 10; // max objects per node
    const int MAX_LEVELS = 9;   // max number of levels to tree

    Quadtree(int level, AABB bounds);

    ~Quadtree();

    bool insert(QuadtreeData qtd);

    void retrieve(std::vector<QuadtreeData>& returnData, AABB box);

    void streamModel(glm::vec3 color);

    int getNodeCount();

private:
    void split();
    void insertRecursive(QuadtreeData qtd);

    int level;
    AABB bounds;
    bool hasChildren = false;
    Quadtree* nodes[4];
    std::vector<QuadtreeData> data;  // list of data in this node
};