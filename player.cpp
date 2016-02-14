#include "player.h"
#include "entityManager.h"

Player::Player(Camera* cam) : speed(SPEED) {
    this->cam = cam;
    timeSinceJump = 10.0f;
    //getTransform()->solid = false;
    getTransform()->visible = false;
    int model = Graphics::registerTransform(false);
    Transform* t = Graphics::getTransform(model);
    getTransform()->parentAll(t);

    t->setPos(0.0f, 1.0f, 0.0f);
    t->setScale(1.0f, 2.0f, 1.0f);
    t->color = glm::vec3(0.2f, 1.0f, 0.7f);
    
}

int Player::getHealth() {
    return health;
}

void Player::update(GLfloat delta) {
    // get movement
    glm::vec3 input = getMovementDir();

    // movement controls
    glm::vec3 xzforward = glm::normalize(glm::cross(cam->worldUp, cam->right));
    if (input != glm::vec3(0.0f, 0.0f, 0.0f)) {
        input = glm::normalize(input);
    }
    if (Input::pressed(sf::Keyboard::Right)) {
        addHealth(5);
    }
    if (Input::pressed(sf::Keyboard::Left)) {
        addHealth(-5);
    }

    // toggle flying
    if (Input::justPressed(sf::Keyboard::Q)) {
        flying = !flying;
    }
    // check jump input
    if (Input::justPressed(sf::Keyboard::Space)) {
        timeSinceJump = 0.0f;
    }

    // check shoot input
    if (Input::justPressed(sf::Keyboard::E)) {
        shoot();
    }

    // jump if in time
    float jumpLenience = 0.2f;
    if (timeSinceJump < jumpLenience) {
        jump();
    }
    timeSinceJump += delta;

    Collider& pt = *getCollider();
    if (flying) {
        pt.gravityMultiplier = 0.0f;
        float flyspeed = speed * 20.0f;
        if (Input::pressed(sf::Keyboard::LControl)) {
            flyspeed *= 3.0f;
        }
        pt.vel = (cam->right * input.x + xzforward * input.z + cam->worldUp * input.y) * flyspeed;
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
        pt.vel += (cam->right * input.x + xzforward * input.z) * accel;

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

void Player::jump() {
    // check if grounded
    Collider& c = *getCollider();
    if (c.grounded && !flying) {
        c.vel.y = JUMPSPEED;
        c.grounded = false;
    }
}

void Player::shoot() {
    glm::vec3 shootPos = getTransform()->getWorldPos();
    shootPos.y += 1.8f;
    EntityManagerInstance->SpawnProjectile(shootPos, getCollider()->vel + cam->forward*80.0f);
}

void Player::addHealth(int amount) {
    health += amount;
    health = std::max(0, std::min(health, MAX_HEALTH));
}

glm::vec3 Player::getMovementDir() {
    // calculate movement direction
    glm::vec3 dir(0.0f, 0.0f, 0.0f);
    if (Input::pressed(sf::Keyboard::W)) {
        dir.z += 1.0f;
    }
    if (Input::pressed(sf::Keyboard::S)) {
        dir.z -= 1.0f;
    }
    if (Input::pressed(sf::Keyboard::A)) {
        dir.x -= 1.0f;
    }
    if (Input::pressed(sf::Keyboard::D)) {
        dir.x += 1.0f;
    }
    if (Input::pressed(sf::Keyboard::LShift)) {
        dir.y -= 1.0f;
    }
    if (Input::pressed(sf::Keyboard::Space)) {
        dir.y += 1.0f;
    }

    float len = static_cast<float>(dir.length());
    if (len > 1)
        dir /= len;//normalize the movement vector so the player doesn't move faster diagonally

    return dir;
}
