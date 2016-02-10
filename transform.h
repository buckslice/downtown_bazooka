#pragma once
#include <glm/glm.hpp>
#include "aabb.h"
#include <iostream>

class Transform {
public:
    glm::vec3 lpos;  //local pos
    glm::vec3 scale;
    //glm::vec3 rot;

    int parent = -1;

    // returns world pos
    glm::vec3 getPos();

private:


};

// will soon replace transform (except the color part should be in a subclass of this new transform)
class BTransform {
public:
    glm::vec3 pos; // local variables
    glm::vec3 scale;
    glm::vec3 rot;

    void setPos(float x, float y, float z);
    void setScale(float x, float y, float z);
    void setRot(float x, float y, float z);

    glm::vec3 color;

    BTransform* parent;

    glm::vec3 getPos(); // world pos
    glm::vec3 getScale();
    glm::vec3 getRot();

    // trying to figure out variadic templates!!! wtffffffffff
    //void parentAll() {
    //}
    //template<typename BTransform*>
    //void parentAll(const BTransform* child) {
    //    child->parent = this;
    //}
    //template<typename BTransform*, typename... Rest>
    //void parentAll(BTransform* first, const Rest&... rest) {
    //    first->parent = this;
    //    parentAll(rest...);
    //}

    //// this example one i found works...
    //void print() {
    //    std::cout << std::endl;
    //}
    //template <typename T> void print(const T& t) {
    //    std::cout << t << std::endl;
    //}
    //template <typename First, typename... Rest> void print(const First& first, const Rest&... rest) {
    //    std::cout << first << ", ";
    //    print(rest...); // recursive call using pack expansion syntax
    //}

    void getWorld(glm::vec3& pos, glm::vec3& rot, glm::vec3& scale);

    BTransform* reset();


private:

};

// should always be parent transform
class PhysicsTransform : public Transform {
public:
    glm::vec3 vel;
    bool grounded = false;
    // should be stored in entity but temporary fix
    // until i get physicsTransform checking in/out better
    bool alive = true;
    float gravityMultiplier = 1.0f;

    // default is 1x1x1 box centered on bottom
    PhysicsTransform(glm::vec3 scale) :
        extmin(glm::vec3(-0.5f, 0.0f, -0.5f)*scale),
        extmax(glm::vec3(0.5f, 1.0f, 0.5f)*scale) {
    }

    AABB getAABB();
    AABB getSwept(float delta);    // uses vel

    inline bool getAffectedByGravity() { return gravityMultiplier != 0.0f; }

private:
    // aabb extents
    glm::vec3 extmin;   // (-0.5f, 0.0f, -0.5f)
    glm::vec3 extmax;   // (0.5f, 2.0f, 0.5f)

};

// class in planning to replace PhysicsTransform
class Collider {
public:
    Transform parent;

    // when register collider just specify whether static or dynamic
    // and it gets put in seperate lists
    //bool isStatic;


private:
    glm::vec3 min;
    glm::vec3 max;

};