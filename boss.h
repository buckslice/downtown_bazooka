#pragma once
#include "entity.h"



class Boss : public ModelEntity {
public:
	Boss(glm::vec3 pos);
	~Boss();

	void update(GLfloat delta) override;
    void render();

	void onCollision(Tag tag, Entity* other) override;

    void addBox(AABB box, glm::vec3 color);

    void playerHitSwitch();

private:
	Transform* body;
	Transform* brain;
	int phase = 0;
    const float maxHealth = 1000.0f;
    float health = maxHealth;

    float vulnerableTimer = 0;

    int switchIndex = -1;
    int switchesHit = 0;
    glm::vec3 oldSwitchColor;

    std::vector<int> boxIndices;
    std::vector<AABB> boxAABBs;
    std::vector<glm::mat4> boxModels;
    std::vector<glm::vec3> boxColors;
};

