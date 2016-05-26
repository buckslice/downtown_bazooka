#include "boss.h"
#include "physics.h"
#include "graphics.h"


Boss::Boss(glm::vec3 pos) {
    body = registerModel();
    body->setPos(pos + glm::vec3(0.0f, 200.0f, 0.0f));
    body->setScale(50.0f, 50.0f, 50.0f);
    brain = registerModel();
    transform->parentAll(body, brain);
    collider->type = ColliderType::FULL;

    Physics::setCollisionCallback(this);
}


Boss::~Boss() {
    std::cout << "BOSS DESTRUCTED";
}

void Boss::update(GLfloat delta) {
    vulnerableTimer -= delta;

    float healthPercent = health / maxHealth;
    if (healthPercent > 0.66f) {
        phase = 0;
    } else if (healthPercent > 0.33f) {
        phase = 1;
    } else {
        phase = 2;
    }

    switch (phase) {
    case 0:
        if (switchIndex < 0) {
            switchIndex = Mth::randRange(0, boxIndices.size());
            // remove a random box (save its aabb and color)
            
            Physics::removeStatic(boxIndices[switchIndex]);
            boxIndices[switchIndex] = Physics::addStatic(boxAABBs[switchIndex], Tag::SWITCH);

            oldSwitchColor = boxColors[switchIndex];
            boxColors[switchIndex] = glm::vec3(1.0f, 1.0f, 0.5f);

            // add particle effect later
        }
        break;
    case 1:

        break;
    case 2:

        break;
    default:
        break;
    }

}

void Boss::render() {
    //std::cout << boxModels.size() << std::endl;
    Graphics::addToStream(Shape::CUBE_GRID, boxModels, boxColors);
}

void Boss::playerHitSwitch() {
    std::cout << "hit switch ";
    if (++switchesHit % 3 == 0) {
        vulnerableTimer = 10.0f;
    }

    // restore switch to previous box
    Physics::removeStatic(boxIndices[switchIndex]);
    boxIndices[switchIndex] = Physics::addStatic(boxAABBs[switchIndex]);
    boxColors[switchIndex] = oldSwitchColor;

    // reroll new random index
    int newIndex = switchIndex;
    while (newIndex == switchIndex) {
        newIndex = Mth::randRange(0, boxIndices.size());
    }
    switchIndex = newIndex;

    // make new switch
    Physics::removeStatic(boxIndices[switchIndex]);
    boxIndices[switchIndex] = Physics::addStatic(boxAABBs[switchIndex], Tag::SWITCH);

    oldSwitchColor = boxColors[switchIndex];
    boxColors[switchIndex] = glm::vec3(1.0f, 1.0f, 0.5f);
}

void Boss::onCollision(Tag tag, Entity* other) {
    if (vulnerableTimer < 0.0f) {
        return; // or something
    }
}

void Boss::addBox(AABB box, glm::vec3 color) {
    boxIndices.push_back(Physics::addStatic(box));;
    boxAABBs.push_back(box);
    boxModels.push_back(box.getModelMatrix());
    boxColors.push_back(color);
}