#include "player.h"

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

	int size = projectiles.size();
	for (int i = 0; i < size; i++) {
		projectiles[i].update(delta);
		if (!projectiles[i].isAlive()) {
			projectiles[i] = projectiles[size - 1];
			projectiles.pop_back();
			size--;
		}
	}

    // jump if in time
    float jumpLenience = 0.2f;
    if (timeSinceJump < jumpLenience) {
        jump();
    }
    timeSinceJump += delta;

    PhysicsTransform& pt = *getTransform();
    if (flying) {
        pt.obeysGravity = false;
        pt.vel = (cam->right * input.x + xzforward * input.z + cam->worldUp * input.y) * speed * 8.0f;
    } else {
        input.y = 0.0f; // ignore this part of input when not flying
        if (pt.vel.y != 0.0f) {
            pt.grounded = false;
        }
        pt.obeysGravity = true;

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
		pt.vel.y = oldy;
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

void Player::shoot() {
	PhysicsTransform& pt = *getTransform();
    glm::vec3 shootPos = pt.getPos();
    shootPos.y += 1.8f;
	Projectile projectile(shootPos, pt.vel + cam->forward*100.0f);
	projectiles.push_back(projectile);
}

void Player::addHealth(int amount) {
    health += amount;
    health = std::max(0, std::min(health, MAX_HEALTH));
}

std::vector<Projectile> Player::getProjectiles() const {
	return projectiles;
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
    return dir;
}
