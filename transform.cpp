
#include "transform.h"

Transform::Transform(glm::vec3 pos, glm::quat rot, glm::vec3 scale,
    glm::vec3 color, Visibility visibility, Shape shape, Transform* parent) :
    pos(pos), rot(rot), scale(scale), color(color), visibility(visibility), shape(shape), parent(parent) {
}

Transform::~Transform() {
}

void Transform::setPos(glm::vec3 pos) {
    this->pos = pos;
    needUpdate = true;
}
void Transform::setPos(float x, float y, float z) {
    pos = glm::vec3(x, y, z);
    needUpdate = true;
}

void Transform::addPos(glm::vec3 add) {
    pos += add;
    needUpdate = true;
}

void Transform::setScale(glm::vec3 scale) {
    this->scale = scale;
    needUpdate = true;
}
void Transform::setScale(float x, float y, float z) {
    scale = glm::vec3(x, y, z);
    needUpdate = true;
}

void Transform::setRot(glm::vec3 euler) {
    rot = glm::quat(euler*DEGREESTORADS);
    needUpdate = true;
}
void Transform::setRot(float x, float y, float z) {
    rot = glm::quat(glm::vec3(x, y, z)*DEGREESTORADS);
    needUpdate = true;
}
void Transform::setRot(glm::quat q) {
    rot = q;
    needUpdate = true;
}

void Transform::setByBounds(AABB bounds) {
    pos = bounds.getCenter();
    scale = bounds.getSize();
    needUpdate = true;
}

// if you dont normalize things rotating fast will starting stretching so weird haha
void Transform::rotate(float angle, glm::vec3 axis) {
    rot *= glm::normalize(glm::angleAxis(angle*DEGREESTORADS, axis));
    needUpdate = true;
}

glm::vec3 Transform::getWorldPos() {
    if (parent != nullptr) {
        return glm::vec3(parent->getModelMatrix() * glm::vec4(pos, 1.0f));
    }
    return pos;
}
glm::vec3 Transform::getWorldScale() {  // world scale?
    if (parent != nullptr) {
        return parent->getWorldScale() * scale;
    }
    return scale;
}

glm::quat Transform::getWorldRot() {
    if (parent != nullptr) {
        return parent->getWorldRot() * rot;
    }
    return rot;
}

glm::mat4 Transform::getModelMatrix() {
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

bool Transform::shouldDraw() {
    if (visibility != Visibility::SHOW_SELF) {
        return false;
    }
    Transform* p = parent;
    while (p != nullptr) {
        if (p->visibility != Visibility::HIDE_ALL) {
            p = p->parent;
        } else {
            return false;
        }
    }
    return true;
}

void Transform::setVisibility(Visibility visibility) {
    this->visibility = visibility;
}
