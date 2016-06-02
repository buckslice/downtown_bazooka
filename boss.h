#pragma once
#include "entity.h"
#include "player.h"


class Boss : public ModelEntity {
public:
	Boss(glm::vec3 pos, Player* player);
	~Boss();

	void update(GLfloat delta) override;
    void render();

	void onCollision(Tag tag, Entity* other) override;

    void addBox(AABB box, glm::vec3 color);

    void playerHitSwitch();

    bool isVulnerable();
    float getHealthPercentage();

private:
	Transform* body;
    Transform* larm;
    Transform* rarm;
    Transform lgun;
    Transform rgun;
    Transform* leye;
    Transform* reye;
    Transform* mouth;
    Transform* lmouth;
    Transform* rmouth;
    Transform* lbrow;
    Transform* rbrow;

    Player* player;

	int phase = 0;
    const float maxHealth = 1000.0f;
    float health = maxHealth;

    glm::vec3 startPosition;
    glm::vec3 targetPosition;
    glm::quat targRot;
    glm::quat currRot;

    float vulnerableTimer = -1.0f;
    const float shotsPerSecond = 1.0f;
    int shotCount = 0;
    float shotTimer = 0.0f;
    bool launchHomingVolley = false;
    bool shootRight = true;
    float bossDyingTime = 5.0f;

    int switchIndex = -1;
    int switchesHit = 0;
    glm::vec3 oldSwitchColor;

    std::vector<int> boxIndices;
    std::vector<AABB> boxAABBs;
    std::vector<glm::mat4> boxModels;
    std::vector<glm::vec3> boxColors;
};

