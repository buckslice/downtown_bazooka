#pragma once

#include <cmath>
#include <vector>
#include <unordered_set>

#include "transform.h"
#include "terrainGenerator.h"


class Physics {
public:
    const float GRAVITY = -30.0f;

    Physics();

    void update(float delta);

    void addStatic(AABB obj);
    void addStatics(const std::vector<AABB>& objs);
    bool checkStatic(AABB obj);
    void clearStatics();
    void clearDynamics();
    static PhysicsTransform* getTransform(int index);
    static int registerDynamic(glm::vec3 scale);

    void printStaticMatrix();

    // fills vectors with model matrices and colors representing all physics colliders
    int getColliderModels(std::vector<glm::mat4>& models, std::vector<glm::vec3>& colors);

    TerrainGenerator* tg;   // ref to terrain for collision detection

private:

    void getLeafs(std::vector<int>& locs, AABB swept);

    // list of static objects (dynamic list is in implementation)
    std::vector<AABB> staticObjects;
    // aabb quadtree that is used to determine what leaf(s) an obj is in
    std::vector<AABB> aabbTree;
    // list for each dynamic object that tells them which leaf(s) they are in
    std::vector<std::vector<int>> dynamicLeafLists;
    // holds lists of statics in each leaf of the quadtree
    std::vector<std::vector<int>> treeMatrix;

    // used to prevent unnecessary additional checks
    std::unordered_set<int> resolvedSet;
    // used to prevent multiple checks on leaf borders
    std::unordered_set<int> checkSet;

};