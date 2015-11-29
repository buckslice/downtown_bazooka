#include "player.h"

int Player::getHealth() {
    return health;
}

void Player::update(GLfloat delta) {

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

    // jump if in time
    float jumpLenience = 0.2f;
    if (timeSinceJump < jumpLenience) {
        jump();
    }
    timeSinceJump += delta;

    // get movement
    glm::vec3 input = getMovementDir();

    // movement controls
    glm::vec3 xzforward = glm::normalize(glm::cross(cam->worldUp, cam->right));
    if (input != glm::vec3(0.0f, 0.0f, 0.0f)) {
        input = glm::normalize(input);
    }

    PhysicsTransform& pt = *getTransform();
    if (flying) {
        pt.vel = (cam->right * input.x + xzforward * input.z + cam->worldUp * input.y) * speed * 8.0f;
    } else {
        input.y = 0.0f; // ignore this part of input when not flying
        if (pt.vel.y != 0.0f) {
            pt.grounded = false;
        }

        GLfloat oldy = pt.vel.y;
        GLfloat accel = pt.grounded ? 10.0f : 2.0f;
        accel *= speed * delta;
        pt.vel.y = 0.0f;
        pt.vel += (cam->right * input.x + xzforward * input.z) * accel;

        if (glm::dot(pt.vel, pt.vel) > speed * speed) {
            pt.vel = glm::normalize(pt.vel) * speed;
        }

        // if no input then apply drag
        if (input == glm::vec3(0.0f)) {
            pt.vel *= (pt.grounded ? .8f : .95f);
            pt.vel *= .9f;
        }
        // gravity 9.81 not right for some reason
        pt.vel.y = oldy + GRAVITY * delta;
    }
}

void Player::jump() {
    // check if grounded
    PhysicsTransform& pt = *getTransform();
    if (pt.grounded && !flying) {
        pt.vel.y = JUMPSPEED;
        pt.grounded = false;
    }
}

void Player::addHealth(int amount) {
    health += amount;
    health = std::max(0, std::min(health, MAX_HEALTH));
}


glm::vec3 Player::getMovementDir() {
    // calculate movement direction
    glm::vec3 dir(0.0f, 0.0f, 0.0f);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Comma) || sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
        dir.z += 1.0f;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::O) || sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
        dir.z -= 1.0f;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
        dir.x -= 1.0f;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::E) || sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
        dir.x += 1.0f;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) {
        dir.y -= 1.0f;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
        dir.y += 1.0f;
    }
    return dir;
}
