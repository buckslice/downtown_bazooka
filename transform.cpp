
#include "transform.h"

// prob add something here
Transform::Transform() {
    reset();
}

void Transform::setPos(glm::vec3 pos) {
    this->pos = pos;
    needUpdate = true;
}
void Transform::setPos(float x, float y, float z) {
    pos = glm::vec3(x, y, z);
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
    if (visibility != VISIBLE) {
        return false;
    }
    Transform* p = parent;
    while (p != nullptr) {
        if (p->visibility != HIDDEN) {
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

// way to reset it since they will be stored in pools
// maybe could just recall constructor?
Transform* Transform::reset() {
    pos = glm::vec3(0.0f);
    rot = glm::quat();
    scale = glm::vec3(1.0f);
    color = glm::vec3(1.0f);
    parent = nullptr;
    visibility = VISIBLE;
    return this;
}