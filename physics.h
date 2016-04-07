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
    int id = -1;
};

class Quadtree;
class Physics {
public:
    const float GRAVITY = -30.0f;

    Physics();

    // progress the physics simulation by delta time
    // center dictates the center of the collision area
    void update(float delta, glm::vec3 center);

    // adds a static to the matrix
    void addStatic(AABB obj);

    // adds multiple statics to the matrix
    void addStatics(const std::vector<AABB>& objs);

    // checks to see if static collides with any in the matrix
    bool checkStatic(AABB obj);

    // clear static object list
    void clearStatics();

    // clear dynamic object list
    //void clearDynamics();

    // returns a Collider pointer from an index
    static Collider* getCollider(int index);

    // sets Entity callback pointer for when collision happens
    static void setCollisionCallback(Entity* entity);

    // registers a collider that will control the movement of
    // the given transform and returns that colliders index
    static int registerDynamic(int transform);

    //static void returnDynamic(int id);

    static void sendOverlapEvent(AABB aabb, CollisionData data);

    // fills vectors with model matrices and colors representing all physics colliders
    int getColliderModels(std::vector<glm::mat4>& models, std::vector<glm::vec3>& colors);

    Terrain* terrainGen;   // ref to terrain for collision detection
private:
    const int SPLIT_COUNT = 7;
    const float MATRIX_SIZE = 1000.0f;

    // pool of dynamic objects
    static std::vector<ColliderData> dynamicObjects;
    static std::vector<int> freeDynamics;
    // list of static objects
    std::vector<AABB> staticObjects;

    static Quadtree* collisionTree;

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


struct QuadtreeData {
    AABB box;   // local copy of aabb to reduce index lookups
    int index;
    bool dynamic;
};

class Quadtree {
public:
    const int MAX_OBJECTS = 10; // max objects per node
    const int MAX_LEVELS = 8;   // max number of levels to tree

    Quadtree(int level, AABB bounds);

    ~Quadtree();

    bool insert(QuadtreeData qtd);

    void retrieve(std::vector<QuadtreeData>& returnData, AABB box);

    void getModel(int& count, std::vector<glm::mat4>& models, std::vector<glm::vec3>& colors);

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