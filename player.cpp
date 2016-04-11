#include "player.h"
#include "entityManager.h"
#include "graphics.h"
#include "audio.h"

Player::Player(Camera* cam) {
    this->cam = cam;
    getTransform()->setVisibility(Visibility::HIDDEN_SELF);

    Transform* model = Graphics::getTransform(Graphics::registerTransform());
    model->setPos(0.0f, 1.5f, 0.0f);
    model->setScale(1.0f, 1.2f, 1.0f);

    Transform* face = Graphics::getTransform(Graphics::registerTransform());
    face->setPos(0.0f, 1.8f, 0.4f);
    face->setScale(0.8f, 0.4f, 0.9f);

    Transform* lleg = Graphics::getTransform(Graphics::registerTransform());
    lleg->setPos(0.35f, 0.5f, 0.0f);
    lleg->setScale(0.25f, 1.0f, 0.25f);

    Transform* rleg = Graphics::getTransform(Graphics::registerTransform());
    rleg->setPos(-0.35f, 0.5f, 0.0f);
    rleg->setScale(0.25f, 1.0f, 0.25f);

    Transform* larm = Graphics::getTransform(Graphics::registerTransform());
    larm->setPos(0.5f, 1.5f, 0.0f);
    larm->setScale(1.0f, 0.25f, 0.25f);

    Transform* rarm = Graphics::getTransform(Graphics::registerTransform());
    rarm->setPos(-0.5f, 1.5f, 0.0f);
    rarm->setScale(1.0f, 0.25f, 0.25f);

    Transform* bazooka = Graphics::getTransform(Graphics::registerTransform());
    bazooka->setPos(0.8f, 1.8f, 0.2f);
    bazooka->setScale(0.5f, 0.5f, 2.0f);

    getTransform()->color = glm::vec3(0.0f, 1.0f, 0.4f);
    getTransform()->parentAllWithColor(model, face, lleg, rleg, larm, rarm, bazooka);
    bazooka->color = glm::vec3(0.25f, 0.0f, 0.5f);

    currRot = targRot = glm::quat();

    Collider* c = getCollider();
    glm::vec3 scale = glm::vec3(1.0f, 2.0f, 1.0f);
    glm::vec3 min = glm::vec3(-0.5f, 0.0f, -0.5f)*scale;
    glm::vec3 max = glm::vec3(0.5f, 1.0f, 0.5f)*scale;
    c->setExtents(min, max);
    c->tag = Tag::PLAYER;
    c->type = ColliderType::FULL;

    Physics::setCollisionCallback(this);
}

void Player::spawn(glm::vec3 spawnPos, bool awake) {
    flying = false;
    health = maxHealth;
    getTransform()->setPos(spawnPos);
    getCollider()->enabled = awake;
}

float Player::getHealth() {
    return health;
}

float Player::getMaxHealth() {
    return maxHealth;
}

float Player::getDamage() {
    return damage;
}

void Player::update(GLfloat delta) {
    isDead = health <= 0.0f;
    if (isDead) {
        getCollider()->enabled = false;
        return;
    }
    bool childrenVisible = cam->getCamDist() > 1.0f;
    getTransform()->setVisibility(childrenVisible ? HIDDEN_SELF : HIDDEN);

    // get movement
    glm::vec3 input = getMovementDir();

    // toggle flying
    if (Input::justPressed(sf::Keyboard::Q)) {
        flying = !flying;
    }
    // check jump input
    if (Input::justPressed(sf::Keyboard::Space)) {
        timeSinceHitJump = 0.0f;
    }

    // check shoot input
    if (Input::justPressed(sf::Keyboard::E)) {

        if (timeSinceShot > 1 / shotsPerSecond) {
            timeSinceShot = 0.0f;
            shoot();
        }
    }

    // jump if in time
    float jumpLenience = 0.2f;
    if (timeSinceHitJump < jumpLenience) {
        jump();
    }
    timeSinceHitJump += delta;
    timeSinceShot += delta;
    invulnTime += delta;

    // cam forward in xz plane
    glm::vec3 xzforward = glm::normalize(glm::cross(cam->worldUp, cam->right));

    // movement vector in xz plane
    glm::vec3 xzmove = cam->right * input.x + xzforward * input.z;
    xzmove.y = 0.0f;
    if (xzmove != glm::vec3(0.0f)) {
        xzmove = glm::normalize(xzmove);
        targRot = glm::rotation(glm::vec3(0.0f, 0.0f, 1.0f), xzmove);
    }

    // slerp keeps the quaternion normalized throughout (lerp looks like shit lol)
    currRot = glm::slerp(currRot, targRot, delta * 8.0f);
    getTransform()->setRot(currRot);

    Collider& pt = *getCollider();
    if (flying) {
        pt.gravityMultiplier = 0.0f;
        float flyspeed = speed * 20.0f;
        if (Input::pressed(sf::Keyboard::LControl)) {
            flyspeed *= 3.0f;
        }
        pt.vel = (xzmove + cam->worldUp * input.y) * flyspeed;
    } else {
        input.y = 0.0f; // ignore this part of input when not flying
        if (pt.vel.y != 0.0f) {
            pt.grounded = false;
        }
        GLfloat oldy = pt.vel.y;

        pt.gravityMultiplier = 1.0f;
        //if (!pt.grounded && Input::pressed(sf::Keyboard::Space) && oldy > 0)//If the player is holding down Space and moving up, then they'll decelerate more slowly
        //	pt.gravityMultiplier = 1.0f;
        //else
        //	pt.gravityMultiplier = 3.0f;

        GLfloat accel = pt.grounded ? 10.0f : 2.0f;
        accel *= speed * delta;
        pt.vel.y = 0.0f;
        pt.vel += xzmove * accel;

        // max vel limit
        if (glm::dot(pt.vel, pt.vel) > speed * speed) {
            pt.vel = glm::normalize(pt.vel) * speed;
        }

        // if no input then apply drag
        if (input == glm::vec3(0.0f)) {
            pt.vel *= (pt.grounded ? .8f : .95f);
            pt.vel *= .9f;
        }
        // gravity 9.81 not right for some reason
        pt.vel.y = oldy;
    }
}

void Player::onCollision(CollisionData data) {
    if ((data.tag == Tag::ENEMY || data.tag == Tag::ENEMY_PROJECTILE) && invulnTime >= 0.5f) {
        AudioInstance->playSound(Resources::get().damageSound);
        addHealth(-5);
        invulnTime = 0.0f;
    }

    if (data.tag == Tag::ITEM) {
        AudioInstance->playSound(Resources::get().itemGetSound);

        Item* i = dynamic_cast<Item*>(data.entity);
        switch (i->type) {
        case ItemType::HEAL:
            addHealth(10);
            break;
        case ItemType::STAMINA:
            maxHealth += 10.0f;
            addHealth(10);
            break;
        case ItemType::STRENGTH:
            damage += 5.0f;
            break;
        case ItemType::AGILITY:
            speed += 5.0f;
            break;
        case ItemType::DEXTERITY:
            shootSpeed += 5.0f;
            shotsPerSecond += 0.5f;
            break;
        }
    }

}

void Player::jump() {
    // check if grounded
    Collider& c = *getCollider();
    if (c.grounded && !flying) {
        AudioInstance->playSound(Resources::get().jumpSound);
        c.vel.y = jumpSpeed;
        c.grounded = false;
        timeSinceHitJump = 1000.0f;
    }
}

void Player::shoot() {
    AudioInstance->playSound(Resources::get().shootSound);
    glm::vec3 shootPos = getTransform()->getWorldPos();
    shootPos.y += 1.8f;
    EntityManagerInstance->SpawnProjectile(shootPos, getCollider()->vel + cam->forward*shootSpeed, true);
}

void Player::addHealth(float amount) {
    health += amount;
    health = std::max(0.0f, std::min(health, maxHealth));
}

// calculate movement direction and return a normal vector pointing in that direction
glm::vec3 Player::getMovementDir() {
    glm::vec3 dir(0.0f, 0.0f, 0.0f);
    if (Input::pressed(sf::Keyboard::W) || Input::pressed(sf::Keyboard::Up)) {
        dir.z += 1.0f;
    }
    if (Input::pressed(sf::Keyboard::S) || Input::pressed(sf::Keyboard::Down)) {
        dir.z -= 1.0f;
    }
    if (Input::pressed(sf::Keyboard::A) || Input::pressed(sf::Keyboard::Left)) {
        dir.x -= 1.0f;
    }
    if (Input::pressed(sf::Keyboard::D) || Input::pressed(sf::Keyboard::Right)) {
        dir.x += 1.0f;
    }
    if (Input::pressed(sf::Keyboard::LShift)) {
        dir.y -= 1.0f;
    }
    if (Input::pressed(sf::Keyboard::Space)) {
        dir.y += 1.0f;
    }

    if (dir != glm::vec3(0.0f)) {
        dir = glm::normalize(dir);
    }

    return dir;
}
