#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <iostream>
#include "mathutil.h"
#include "pool.h"

// will soon replace transform (except the color part should be in a subclass of this new transform)
class Transform {
public:
    Transform();

    // set local position of transform
    void setPos(glm::vec3 pos);
    // set local position of transform
    void setPos(float x, float y, float z);
    // set local scale of transform
    void setScale(glm::vec3 scale);
    // set local scale of transform
    void setScale(float x, float y, float z);
    // set local rotation of transform
    void setRot(glm::vec3 euler);
    // set local rotation of transform
    void setRot(float x, float y, float z);
    // local rotation around axis by angle in degrees
    void rotate(float angle, glm::vec3 axis);
    
    // returns world pos of transform (transformed by parent matrices)
    glm::vec3 getWorldPos();

    // returns world scale of transform
    glm::vec3 getWorldScale();

    // get local position
    inline glm::vec3 getPos() { return pos; }
    // get local scale
    inline glm::vec3 getScale() { return scale; }

    // get model matrix for rendering
    glm::mat4 getModelMatrix();

    // variadic template mass parenting function
    // couldn't get it working in implementation
    // modifies local position/scale based on scale of parent
    template<typename T>
    void parentAll(T* first) {  // base case
        first->parent = this;
        first->pos /= scale;
        first->scale /= scale;
    }
    template<typename T, typename... R>
    void parentAll(T* first, const R&... rest) { // recursive
        first->parent = this;
        first->pos /= scale;
        first->scale /= scale;
        parentAll(rest...);
    }

    // way to reset it since they will be stored in pools
    // maybe could just recall constructor?
    Transform* reset();

    glm::vec3 color;
    bool visible = true;
    // maybe when you register you specify solid or not and it gets put on different list
    bool solid = true;

private:
    glm::vec3 pos;
    glm::quat rot;
    glm::vec3 scale;

    glm::mat4 model;           // local model matrix
    // may be worth caching parents matrix too like benny does

    bool needUpdate = true;    // set to true whenever model matrix needs to be recalculated

    // should switch this to index later once more work is done with pools / slotmaps
    Transform* parent = nullptr;

};
