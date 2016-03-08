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

class Physics {
public:
    const float GRAVITY = -30.0f;

    Physics();

    // progress the physics simulation
    void update(float delta);

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

    // builds collision tree centered around a point
    void generateCollisionMatrix(glm::vec3 center);

    // fills vectors with model matrices and colors representing all physics colliders
    int getColliderModels(std::vector<glm::mat4>& models, std::vector<glm::vec3>& colors);

    Terrain* tg;   // ref to terrain for collision detection

private:
    const int SPLIT_COUNT = 7;
    const float MATRIX_SIZE = 1000.0f;

    // searches tree and returns a list of leaf indices AABB collides with
    // see notes below for ideas on how to improve lookup speed
    static void getLeafs(std::vector<int>& locs, AABB aabb);

    // returns number of dynamics intersecting with collision matrices
    int getNumberOfIntersections();

    // pool of dynamic objects
    static std::vector<ColliderData> dynamicObjects;
    static std::vector<int> freeDynamics;
    // list of static objects
    std::vector<AABB> staticObjects;

    // aabb quadtree that is used to determine what leaf(s) an obj is in
    static std::vector<AABB> aabbTree;
    // list for each dynamic object that tells them which leaf(s) they are in
    std::vector<std::vector<int>> dynamicLeafLists;

    // a list for each leaf in the aabbTree containing a list of each object in that leaf
    // better name would probably be objectsInLeaves or something but yolo
    static std::vector<std::vector<int>> dynamicMatrix;
    std::vector<std::vector<int>> staticMatrix;

    // used to prevent unnecessary additional checks
    std::unordered_set<int> staticResolvedSet;
    std::unordered_set<int> dynamicResolvedSet;

    // used to prevent multiple checks on leaf borders
    std::unordered_set<int> staticCheckSet;
    std::unordered_set<int> dynamicCheckSet;

};


// NOTES

// notes from thread
// The rest of my algorithm is as optimal as I could make it.I have an 8 - level quad -
// tree with nodes linear in RAM for best caching. Insertion into any node is nearly instantaneous,
// as calculating the depth and then the node in that depth for a given AABB is only a few
// instructions total(as apposed to the naive implementation which would have me starting at the top
// and searching for the best - fit node with a bunch of AABB tests).

// im doing naive currenly
// could probably just calculate size of each leaf node
// and check ur aabb based on that
// check each corner of your box and add you to each of those leaves
// only thing is i think huge buildings may occupy more than 4 leaves since they are so huge