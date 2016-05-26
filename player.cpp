#include "player.h"
#include "entityManager.h"
#include "graphics.h"
#include "audio.h"
#include "game.h"
#include <iomanip>

Player::Player(Camera* cam) {
    this->cam = cam;
    transform->setVisibility(Visibility::HIDE_SELF);

    Transform* model = Graphics::registerTransform();
    model->setPos(0.0f, 1.5f, 0.0f);
    model->setScale(1.0f, 1.2f, 1.0f);

    Transform* face = Graphics::registerTransform();
    face->setPos(0.0f, 1.8f, 0.4f);
    face->setScale(0.8f, 0.4f, 0.9f);

    Transform* lleg = Graphics::registerTransform();
    lleg->setPos(0.35f, 0.5f, 0.0f);
    lleg->setScale(0.25f, 1.0f, 0.25f);

    Transform* rleg = Graphics::registerTransform();
    rleg->setPos(-0.35f, 0.5f, 0.0f);
    rleg->setScale(0.25f, 1.0f, 0.25f);

    Transform* larm = Graphics::registerTransform();
    larm->setPos(0.5f, 1.5f, 0.0f);
    larm->setScale(1.0f, 0.25f, 0.25f);

    Transform* rarm = Graphics::registerTransform();
    rarm->setPos(-0.5f, 1.5f, 0.0f);
    rarm->setScale(1.0f, 0.25f, 0.25f);

    Transform* bazooka = Graphics::registerTransform();
    bazooka->setPos(0.8f, 1.8f, 0.2f);
    bazooka->setScale(0.5f, 0.5f, 2.0f);

    transform->color = glm::vec3(0.0f, 1.0f, 0.4f);
    transform->parentAllWithColor(model, face, lleg, rleg, larm, rarm, bazooka);
    bazooka->color = glm::vec3(0.25f, 0.0f, 0.5f);

    currRot = targRot = glm::quat();

    glm::vec3 scale = glm::vec3(1.0f, 2.0f, 1.0f);
    glm::vec3 min = glm::vec3(-0.5f, 0.0f, -0.5f)*scale;
    glm::vec3 max = glm::vec3(0.5f, 1.0f, 0.5f)*scale;
    collider->setExtents(min, max);
    collider->tag = Tag::PLAYER;
    collider->type = ColliderType::FULL;

    Physics::setCollisionCallback(this);

}

void Player::spawn(glm::vec3 spawnPos, bool enabled) {
    flyMode = false;
    health = maxHealth;
    burnTime = 0.0f;
    boostParticleTime = 0.0f;
    transform->setPos(spawnPos);
    collider->enabled = enabled;
}

float Player::getHealth() const {
    return health;
}

float Player::getMaxHealth() const {
    return maxHealth;
}

float Player::getDamage() const {
    return attackDamage;
}

bool Player::isDead() const {
    return health <= 0.0f;
}

void Player::addHealth(float amount) {
    health += amount;
    health = std::max(0.0f, std::min(health, maxHealth));
}


void Player::update(GLfloat delta) {
    if (isDead()) {
        collider->enabled = false;
        return;
    }
    updateTimers(delta);

    glm::vec3 targetDir = checkInputs();

    updateModel(targetDir, delta);

    if (flyMode) {   // debug mode, execute this and return
        flyModeMovement(targetDir);
        return;
    }

    calculateBurnDamage(delta);

    checkClampXZVel();

    checkJumpAndBoost();

    calculateMovement(targetDir, delta);

}

void Player::updateTimers(float delta) {
    // update timer variables
    timeSinceHitJump += delta;
    timeSinceGrounded += delta;
    timeSinceShot += delta;
    invulnTime += delta;
    burnTime -= delta;
    boostTimer -= delta;
    boostParticleTime -= delta;
}

void Player::shoot() {
    AudioInstance->playSound(Resources::get().shootSound);
    glm::vec3 shootPos = transform->getWorldPos();
    shootPos.y += 1.8f;
    //EntityManagerInstance->SpawnProjectile(shootPos, collider->vel + cam->forward*shootSpeed, true);
    EntityManagerInstance->SpawnProjectile(shootPos, cam->forward*shootSpeed, true);
}

glm::vec3 Player::checkInputs() {
    // toggle flying
    if (Input::justPressed(sf::Keyboard::Q)) {
        burnTime = 0.0f;
        boostParticleTime = 0.0f;
        flyMode = !flyMode;
    }
    // check jump input
    if (Input::justPressed(sf::Keyboard::Space)) {
        timeSinceHitJump = 0.0f;
    }

    // check shoot input
    if (Input::pressed(sf::Keyboard::E) || Input::pressed(sf::Mouse::Button::Right)) {
        if (timeSinceShot > 1 / shotsPerSecond) {
            timeSinceShot = 0.0f;
            shoot();
        }
    }

    if (Input::pressed(sf::Keyboard::BackSpace)) {
        glm::vec3 p = transform->getWorldPos();
        std::cout << std::setprecision(2) << std::fixed;
        std::cout << p.x << " " << p.y << " " << p.z << std::endl;
    }

    // calculate movement vector in xz plane
    glm::vec3 input = getMovementDir(); // input from key presses
    glm::vec3 xzCamForward = glm::normalize(glm::cross(cam->worldUp, cam->right)); // cam forward in xz plane
    glm::vec3 targetDir = cam->right * input.x + xzCamForward * input.z;  //normalized target movement direction
    targetDir.y = 0.0f;    // just to make sure
    if (targetDir != glm::vec3(0.0f)) {
        targetDir = glm::normalize(targetDir);
    }

    return targetDir;
}

void Player::updateModel(glm::vec3 targetDir, float delta) {
    // only update model rotation if nonzero movement vector
    if (targetDir != glm::vec3(0.0f)) {
        targRot = glm::normalize(glm::rotation(glm::vec3(0.0f, 0.0f, 1.0f), targetDir));
    }
    // slerp keeps the quaternion normalized throughout
    currRot = glm::slerp(currRot, targRot, delta * 8.0f);
    transform->setRot(currRot);

    // set your model visible based on how zoomed in camera is
    bool childrenVisible = cam->getCamDist() > 1.0f;
    transform->setVisibility(childrenVisible ? Visibility::HIDE_SELF : Visibility::HIDE_ALL);

}

void Player::flyModeMovement(glm::vec3 targetDir) {
    collider->gravityMultiplier = 0.0f;
    float flyspeed = speed * 20.0f;
    if (Input::pressed(sf::Keyboard::LControl)) {
        flyspeed *= 3.0f;
    }
    float yInput = 0.0f;
    if (Input::pressed(sf::Keyboard::LShift)) {
        yInput -= 1.0f;
    }
    if (Input::pressed(sf::Keyboard::Space)) {
        yInput += 1.0f;
    }
    collider->vel = (targetDir + cam->worldUp * yInput) * flyspeed;
}

bool Player::recentlyGrounded() {
    return timeSinceGrounded < groundedLenience;
}

void Player::calculateBurnDamage(float delta) {
    if (collider->onTerrain && Game::isGroundLava()) {
        burnTime = 0.5f;
    }
    if (burnTime > 0.0f) {
        AudioInstance->playSoundSingle(Resources::get().burningSound);

        glm::vec3 rvel = glm::vec3(Mth::randUnit(), Mth::rand01() + 0.5f, Mth::randUnit()) * 10.0f;
        EntityManagerInstance->SpawnParticle(ParticleType::FIRE, transform->getWorldPos(), rvel, 5.0f);
        addHealth(-delta*20.0f);
    }
}

// work around for shortcoming with physics resolution
// problem is physics doesnt zero your velocity in the direction of things you hit (like walls)
// so if you slam into a corner of two buildings really fast you will be stuck for a bit while
// your speed cools down, so what this does is check if your last x and z positions are almost equal
// to your current then you set the x and z components of velocity to zero
void Player::checkClampXZVel() {
    glm::vec3 curPos = transform->getPos();
    curPos.y = 0.0f;
    glm::vec3 diff = curPos - oldPos;
    if (glm::dot(diff, diff) < 0.00001f) {
        //std::cout << "decimating";
        collider->vel.x = 0.0f;
        collider->vel.z = 0.0f;
    }
    // save current position
    oldPos = curPos;
}

void Player::checkJumpAndBoost() {
    // jump if able
    if (timeSinceHitJump < jumpLenience && recentlyGrounded()) {
        AudioInstance->playSound(Resources::get().jumpSound);
        collider->vel.y = jumpSpeed;
        collider->grounded = false;
        timeSinceGrounded = 10.0f;
        timeSinceHitJump = 10.0f;
    }
    if (collider->grounded) {   // set time since grounded
        timeSinceGrounded = 0.0f;
    }

    if (Input::justPressed(sf::Mouse::Button::Left) && recentlyGrounded() && boostTimer < 0.0f) {
        // set speed in negative camera facing direction
        collider->vel += -cam->forward * boostSpeed;

        boostTimer = boostCooldown;
        boostParticleTime = 1.0f;

        AudioInstance->playSound(Resources::get().boostSound);

        addHealth(-1.0f);  // slightly hurt player
        timeSinceGrounded = 10.0f;

        // spawn a bunch of particles
        glm::vec3 pos = transform->getWorldPos();
        for (int i = 0; i < 200; ++i) {
            glm::vec3 r = Mth::randInsideSphere(40.0f);
            r.y *= 0.75f;
            EntityManagerInstance->SpawnParticle(BOOST, pos, r);
        }
    }

    // continue launching boost particles for a bit
    if (boostParticleTime > 0.0f) {
        glm::vec3 pos = transform->getWorldPos();
        float s = std::max(0.1f, boostParticleTime / 1.0f);
        for (int i = 0; i < 2; ++i) {
            EntityManagerInstance->SpawnParticle(BOOST, pos, Mth::randInsideSphere(20.0f * s), 0.0f, glm::vec3(s), false);
        }
    }
}

void Player::calculateMovement(glm::vec3 targetDir, float delta) {
    collider->gravityMultiplier = 1.0f; // ensure normal gravity

    // save y velocity and zero it
    float oldY = collider->vel.y;
    collider->vel.y = 0.0f;

    // save max value out of square speed and square magnitude of velocity
    float sqrMagSave = glm::max(speed*speed, glm::dot(collider->vel, collider->vel));

    // acceleration based on groundedness
    float accel = recentlyGrounded() ? 75.0f : 35.0f;

    // add to velocity
    collider->vel += targetDir * accel * delta;

    // if velocity is over sqrMagSave that means it was both over
    // character speed limit and the magnitude of previous velocity
    // so clamp it to sqrMagSave
    if (glm::dot(collider->vel, collider->vel) > sqrMagSave) {
        collider->vel = glm::normalize(collider->vel) * glm::sqrt(sqrMagSave);
    }

    // if still over character speed limit then apply drag to velocity
    float drag = 0.0f;
    bool noInput = targetDir == glm::vec3(0.0f);
    if (glm::dot(collider->vel, collider->vel) > speed * speed) { // apply light drag if over speed limit
        drag = recentlyGrounded() ? 0.5f : 0.025f;
        if (noInput) drag *= 2.0f; // increase drag if no inputs are pressed
    } else if (noInput) { // else if within speed limit and no input then apply harder drag
        drag = recentlyGrounded() ? 12.0f : 3.0f;
    }
    drag = Mth::saturate(drag * delta);
    collider->vel -= collider->vel * drag;

    // restore old y
    collider->vel.y = oldY;
}

void Player::onCollision(Tag tag, Entity* other) {
    switch (tag) {
    case Tag::HEALER: {
        if (collider->grounded && !collider->onTerrain && health < maxHealth) {
            AudioInstance->playSoundSingle(Resources::get().healingSound);
            addHealth(20.0f * Game::deltaTime());
            glm::vec2 p = glm::normalize(Mth::randomPointInCircle(1.0f))*3.0f;
            glm::vec3 rp = transform->getWorldPos() + glm::vec3(p.x, 0.0f, p.y);
            EntityManagerInstance->SpawnParticle(ParticleType::HEAL, rp, glm::vec3(0.0f), 1.0f);
        }
    }break;
    case Tag::SWITCH: {
        Boss* b = EntityManagerInstance->getBoss();
        if (b != nullptr) {
            b->playerHitSwitch();
        }
    }break;
    case Tag::ENEMY:
    case Tag::ENEMY_PROJECTILE:
        if (invulnTime >= 0.5f) {
            AudioInstance->playSound(Resources::get().damageSound);
            addHealth(-5);
            invulnTime = 0.0f;
        }
        break;
    case Tag::ITEM:
        AudioInstance->playSound(Resources::get().itemGetSound);

        Item* i = dynamic_cast<Item*>(other);
        switch (i->type) {
        case ItemType::HEAL:
            addHealth(30);
            break;
        case ItemType::STAMINA:
            maxHealth += 10.0f;
            addHealth(10);
            break;
        case ItemType::STRENGTH:
            attackDamage += 5.0f;
            break;
        case ItemType::AGILITY:
            speed += 5.0f;
            break;
        case ItemType::DEXTERITY:
            shootSpeed += 5.0f;
            shotsPerSecond += 0.5f;
            break;
        }
        break;
    }

}

// calculate movement direction and return a normalized vector pointing in that direction
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
    if (dir != glm::vec3(0.0f)) {
        dir = glm::normalize(dir);
    }

    return dir;
}
