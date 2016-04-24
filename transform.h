#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <iostream>
#include "mathutil.h"
#include "aabb.h"

enum class Visibility {
    SHOW_SELF,       // show yourself
    HIDE_SELF,       // hide just yourself
    HIDE_ALL         // hide yourself and your children
};

enum class Shape {
    CUBE_SOLID,
    CUBE_GRID,
    PYRAMID
};

class Transform {
public:
    Transform(
        glm::vec3 pos = glm::vec3(0.0f),
        glm::quat rot = glm::quat(),
        glm::vec3 scale = glm::vec3(1.0f),
        glm::vec3 color = glm::vec3(1.0f),
        Visibility visibility = Visibility::SHOW_SELF,
        Shape shape = Shape::CUBE_GRID,
        Transform* parent = nullptr
        );

    ~Transform();

    // set local position of transform
    void setPos(glm::vec3 pos);
    // set local position of transform
    void setPos(float x, float y, float z);
    // adds to position of transform
    void addPos(glm::vec3 add);

    // set local scale of transform
    void setScale(glm::vec3 scale);
    // set local scale of transform
    void setScale(float x, float y, float z);

    // set local rotation of transform by euler vector
    void setRot(glm::vec3 euler);
    // set local rotation of transform by euler values
    void setRot(float x, float y, float z);
    // set local rotation of transform by quaternion
    void setRot(glm::quat q);

    // sets up position and scale from provided AABB
    void setByBounds(AABB bounds);

    // rotate around axis by angle in degrees (local)
    void rotate(float angle, glm::vec3 axis);

    // returns world pos of transform (transformed by parent matrices)
    glm::vec3 getWorldPos() const;

    // returns world scale of transform
    glm::vec3 getWorldScale() const;

    // returns world rotation of transform
    glm::quat getWorldRot() const;

    // get local position
    inline glm::vec3 getPos() const { return pos; }
    // get local scale
    inline glm::vec3 getScale() const { return scale; }
    // get local rotation
    inline glm::quat getRot() const { return rot; }

    // get model matrix for rendering
    glm::mat4 getModelMatrix();

    bool shouldDraw() const;

    void setVisibility(Visibility vis);

    // variadic template mass parenting function
    // kind of annoying that you need two functions, maybe theres better way
    // modifies local position/scale based on scale of parent
    // so you can define everything in world space
    template<typename T>
    void parentAll(T* first) {  // base case
        first->parent = this;
        first->pos /= getWorldScale();
        first->scale /= getWorldScale();
    }
    template<typename T, typename... R>
    void parentAll(T* first, const R&... rest) { // recursive
        first->parent = this;
        first->pos /= getWorldScale();
        first->scale /= getWorldScale();
        parentAll(rest...);
    }

    // another parenting function that also assigns color to children
    template<typename T>
    void parentAllWithColor(T* first) {  // base case
        parentAll(first);
        first->color = color;
    }
    template<typename T, typename... R>
    void parentAllWithColor(T* first, const R&... rest) { // recursive
        parentAll(first);
        first->color = color;
        parentAllWithColor(rest...);
    }

    glm::vec3 color;
    Shape shape;

private:
    glm::vec3 pos;
    glm::vec3 scale;
    glm::quat rot;

    glm::mat4 model;           // local model matrix
    // may be worth caching parents matrix too like benny does

    bool needUpdate = true;    // set to true whenever model matrix needs to be recalculated

    Visibility visibility;

    Transform* parent = nullptr;

};
