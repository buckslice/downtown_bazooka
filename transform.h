#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include "aabb.h"
#include <iostream>
#include "mathutil.h"
#include "pool.h"

//class Transform {
//public:
//    glm::vec3 lpos;  //local pos
//    glm::vec3 scale;
//    //glm::vec3 rot;
//
//    int parent = -1;
//
//    // returns world pos
//    glm::vec3 getPos();
//
//private:
//
//
//};

// will soon replace transform (except the color part should be in a subclass of this new transform)
class BTransform {
public:

    // should add a constructor probs lol
    BTransform() {
    }

    // way to reset it since they will be stored in pools
    // maybe could just recall constructor?
    BTransform* reset() {
        pos = glm::vec3(0.0f);
        rot = glm::quat();
        scale = glm::vec3(1.0f);
        color = glm::vec3(1.0f);
        parent = nullptr;
        return this;
    }

    // set local position of transform
    void setPos(glm::vec3 pos) {
        this->pos = pos;
        needUpdate = true;
    }
    // set local position of transform
    void setPos(float x, float y, float z) {
        pos = glm::vec3(x, y, z);
        needUpdate = true;
    }
    // set local scale of transform
    void setScale(glm::vec3 scale) {
        this->scale = scale;
        needUpdate = true;
    }
    // set local scale of transform
    void setScale(float x, float y, float z) {
        scale = glm::vec3(x, y, z);
        needUpdate = true;
    }
    // set local rotation of transform
    void setRot(glm::vec3 euler) {
        rot = glm::quat(euler*DEGREESTORADS);
        needUpdate = true;
    }
    // set local rotation of transform
    void setRot(float x, float y, float z) {
        rot = glm::quat(glm::vec3(x, y, z)*DEGREESTORADS);
        needUpdate = true;
    }
    // local rotation around axis by angle in degrees
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

    // returns world pos of transform (transformed by parent matrices)
    glm::vec3 getWorldPos() {
        if (parent != nullptr) {
            return glm::vec3(parent->getModelMatrix() * glm::vec4(pos, 1.0f));
        }
        return pos;
    }

    // not sure about this one for scale, or how / if i want to do rotations (lol who cares amirite?)

    // returns world scale of transform
    glm::vec3 getWorldScale() {  // world scale?
        if (parent != nullptr) {
            return parent->getWorldScale() * scale;
        }
        return scale;
    }

    // get local position
    inline glm::vec3 getPos() { return pos; }
    // get local scale
    inline glm::vec3 getScale() { return scale; }

    // get model matrix for rendering
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

    // some temp stuff for testing
    // will later be put into actual boxtransform class or something
    glm::vec3 color;
    bool visible;

    // variadic template mass parenting function
    template<typename T>
    void parentAll(T* first) {   // base case
        first->parent = this;
    }
    template<typename T, typename... R>
    void parentAll(T* first, const R&... rest) { // recusive function generation
        first->parent = this;
        parentAll(rest...);
    }

private:
    glm::vec3 pos;
    glm::quat rot;
    glm::vec3 scale;

    glm::mat4 model;           // local model matrix
    // may be worth caching parents matrix too like benny does

    bool needUpdate = true;    // set to true whenever model matrix needs to be recalculated

    BTransform* parent;

    // just make sure parent frees children as well when he is freed?

    // maybe keep reference to object and check if id is <0 before continueing?

    // maybe have pointer to dynamic vector of children you can update and set their parents to null?

    // would prob have to use slotmap for this as well
    // should just never delete parents only put them to sleep or something i dunno
    //int parent;
    //Pool<BTransform>* parentLoc;    // which pool parent belongs to
    //BTransform* getParent() {
    //    parentLoc->get(parent)->data;
    //}

};

// should always be parent transform
class PhysicsTransform : public BTransform {
public:
    glm::vec3 vel;
    bool grounded = false;
    // should be stored in entity but temporary fix
    // until i get physicsTransform checking in/out better
    //bool alive = true;
    float gravityMultiplier = 1.0f;

    // default is 1x1x1 box centered on bottom
    PhysicsTransform() {
        setExtents(glm::vec3(1.0f));
    }

    void setExtents(glm::vec3 scale);

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