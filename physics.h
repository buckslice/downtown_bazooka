#pragma once

#include <cmath>
#include <vector>
#include <unordered_set>

#include "transform.h"
#include "collider.h"
#include "terrain.h"

struct leafObject {
    int index;      // index points to object in a list
    bool dynamic;   // determines which list index is for
};


class Physics {
public:
    const float GRAVITY = -30.0f;

    Physics();

    void update(float delta);

    void addStatic(AABB obj);
    void addStatics(const std::vector<AABB>& objs);
    bool checkStatic(AABB obj);
    void clearStatics();
    //void clearDynamics();
    
    static Collider* getCollider(int index);
    static int registerDynamic(int transform);
    //static void returnDynamic(int id);

    // builds collision tree centered around a point
    void generateCollisionMatrix(glm::vec3 center);

    // fills vectors with model matrices and colors representing all physics colliders
    int getColliderModels(std::vector<glm::mat4>& models, std::vector<glm::vec3>& colors);

    Terrain* tg;   // ref to terrain for collision detection

private:
    const int SPLIT_COUNT = 6;
    const float MATRIX_SIZE = 1000.0f;

    void getLeafs(std::vector<int>& locs, AABB swept);


    // list of static objects (dynamic list is in implementation)
    std::vector<AABB> staticObjects;
    // aabb quadtree that is used to determine what leaf(s) an obj is in
    std::vector<AABB> aabbTree;
    // list for each dynamic object that tells them which leaf(s) they are in
    std::vector<std::vector<int>> dynamicLeafLists;

    // a list for each leaf in the aabbTree containing a list of each object in that leaf
    // better name would probably be objectsInLeaves or something lol
    // but superMatrix sounds BADASS
    std::vector<std::vector<leafObject>> superMatrix;
    //std::vector<std::vector<int>> staticMatrix;

    // used to prevent unnecessary additional checks
    std::unordered_set<int> staticResolvedSet;
    std::unordered_set<int> dynamicResolvedSet;

    // used to prevent multiple checks on leaf borders
    std::unordered_set<int> staticCheckSet;
    std::unordered_set<int> dynamicCheckSet;

};