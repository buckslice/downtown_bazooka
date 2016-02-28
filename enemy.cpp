#include "enemy.h"
#include "graphics.h"

Enemy::Enemy(int player, glm::vec3 start, glm::vec3 scale, glm::vec3 color) : Entity(start, scale) {
    this->player = player;  // player transform
    speed = Mth::rand01() * 5.0f + 5.0f;
    jumpVel = Mth::rand01() * 10.0f + 20.0f;

    Transform* model = Graphics::getTransform(Graphics::registerTransform(false));
    model->setPos(0.0f, scale.y / 2.0f, 0.0f);
    model->setScale(scale);

    getTransform()->visible = false;
    getTransform()->color = color;
    getTransform()->parentAllWithColor(model);

    getCollider()->type = ColliderType::FULL;
}

void Enemy::update(GLfloat delta) {
    Collider& col = *getCollider();

    jumpTimer -= delta;
    if (col.grounded && jumpTimer < 0.0f) {
        col.vel.y = jumpVel;
        col.grounded = false;
        jumpTimer += Mth::rand01() * 10.0f + 2.0f;
    }

    glm::vec3 dirToPlayer = Graphics::getTransform(player)->getWorldPos() - getTransform()->getWorldPos();
    dirToPlayer.y = 0.0f;
    if (dirToPlayer != glm::vec3(0.0f)) {
        dirToPlayer = glm::normalize(dirToPlayer) * speed;
    }

    col.vel.x = dirToPlayer.x;
    col.vel.z = dirToPlayer.z;

}