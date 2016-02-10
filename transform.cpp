
#include "transform.h"
#include "physics.h"

// returns world pos of transform
glm::vec3 Transform::getPos() {
    if (parent >= 0) {
        return Physics::getTransform(parent)->getPos() + lpos;
    }
    return lpos;
}

glm::vec3 BTransform::getPos() {
    if (parent != nullptr) {
        return parent->getPos() + pos;
    }
    return pos;
}

glm::vec3 BTransform::getRot() {
    if (parent != nullptr) {
        return parent->getRot() + rot;
    }
    return rot;
}

glm::vec3 BTransform::getScale() {
    if (parent != nullptr) {
        return parent->getScale() * scale;
    }
    return scale;
}

void BTransform::setPos(float x, float y, float z) {
    pos = glm::vec3(x, y, z);
}
void BTransform::setRot(float x, float y, float z) {
    rot = glm::vec3(x, y, z);
}
void BTransform::setScale(float x, float y, float z) {
    scale = glm::vec3(x, y, z);
}

//template<typename BTransform*>
//void BTransform::parentAll() {
//}
//template<typename BTransform*, typename ... Rest>
//void BTransform::parentAll(BTransform* first, Rest ... rest){
//    first->parent = this;
//    parentAll(rest...);
//}

// gets world transform with less checks and method calls
// make sure to pass in glm::vec3(1.0f) for scale and zero for others
void BTransform::getWorld(glm::vec3& pos, glm::vec3& rot, glm::vec3& scale) {
    if (parent != nullptr) {
        parent->getWorld(pos, rot, scale);
    }
    pos += this->pos;
    rot += this->rot;
    scale *= this->scale;
}

BTransform* BTransform::reset() {
    pos = glm::vec3(0.0f);
    rot = glm::vec3(0.0f);
    scale = glm::vec3(1.0f);
    color = glm::vec3(1.0f);
    parent = nullptr;
    return this;
}

// set world pos (updates local pos based on parent stuff)

AABB PhysicsTransform::getAABB() {
    return AABB(glm::vec3(lpos + extmin), glm::vec3(lpos + extmax));
}

AABB PhysicsTransform::getSwept(float delta) {
    return AABB::getSwept(getAABB(), vel * delta);
}