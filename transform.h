#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
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

    BTransform() {

    }

    // setters
    void setPos(glm::vec3 pos) {
        this->pos = pos;
        needUpdate = true;
    }
    void setPos(float x, float y, float z) {
        pos = glm::vec3(x, y, z);
        needUpdate = true;
    }
    void setScale(glm::vec3 scale) {
        this->scale = scale;
        needUpdate = true;
    }
    void setScale(float x, float y, float z) {
        scale = glm::vec3(x, y, z);
        needUpdate = true;
    }
    void setRot(glm::vec3 euler) {
        rot = glm::quat(euler);
        needUpdate = true;
    }
    void setRot(float x, float y, float z) {
        rot = glm::quat(glm::vec3(x, y, z));
        needUpdate = true;
    }

    void rotate(float angle, glm::vec3 axis) {
        rot *= glm::angleAxis(angle, axis);
        needUpdate = true;
    }

    // investigate how benny does it 
    // with gettransformedpos probably which takes into account scale of your parent
    // and such
    // getters (world space)
    // local would just be private variables
    // should also research where to inline / if it just happens automatically anyways lol

    // returns world pos correctly transformed by parents transform
    glm::vec3 getPos() {
        if (parent != nullptr) {
            return glm::vec3(parent->getModelMatrix() * glm::vec4(pos, 1.0f));
        }
        return pos;
    }


    // local pos getters should return reference so they can be set too? maybe?

    // not sure about this one for scale, or how / if i want to do rotations (lol who cares amirite?)
    glm::vec3 getScale() {  // world scale?
        if (parent != nullptr) {
            return parent->getScale() * scale;
        }
        return scale;
    }


    glm::mat4 getModelMatrix() {
        if (needUpdate) {   // recalculate model matrix if needs update
            model = glm::mat4();
            model = glm::translate(model, pos);
            model *= glm::toMat4(rot);
            model = glm::scale(model, scale);
            needUpdate = false;
        }

        if (parent != nullptr) {
            return parent->getModelMatrix() * model;
        }

        return model;
    }


    glm::vec3 color;

    // way to reset it since they will be stored in pools
    // maybe could just recall constructor?
    BTransform* reset();


    // variadic template mass parenting function
    template<typename T>
    void parentAll(T first) {   // base case
        first->parent = this;
    }
    template<typename T, typename... R>
    void parentAll(T first, const R&... rest) { // recusive function generation
        first->parent = this;
        parentAll(rest...);
    }

    // does same thing pretty much but in different way (saving for next commit)
    //struct sink {
    //    template<typename ...T> sink(T && ...) {}
    //};
    //template<typename ...T>
    //void parentAll(T... tforms) {
    //    sink{ (tforms->parent = this)... };
    //}


private:
    glm::vec3 pos;
    glm::quat rot;
    glm::vec3 scale;

    glm::mat4 model;
    bool needUpdate = true;    // set to true whenever model matrix needs to be recalculated

    BTransform* parent;

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

//// class in planning to replace PhysicsTransform
//class Collider {
//public:
//    Transform parent;
//
//    // when register collider just specify whether static or dynamic
//    // and it gets put in seperate lists
//    //bool isStatic;
//
//
//private:
//    glm::vec3 min;
//    glm::vec3 max;
//
//};