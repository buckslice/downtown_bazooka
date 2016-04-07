#pragma once
#include "entity.h"

enum class ItemType {
	HEAL, // heals for a portion of the player's health
	STAMINA, // permanently increases the player's maximum health
	STRENGTH, // permanently increases the amount of damage each of the player's shots does
	AGILITY, // permanently increases the player's movement speed and jump height
	DEXTERITY, // permanently increases the amount of shots the player can fire per second
	COUNT // just as a count for the number of elements in this enum
};

class Item : public Entity {
public:
	Item();
	~Item();

	void update(GLfloat delta) override;

	void onCollision(CollisionData data) override;

	void init(int id, float rotSpeed, glm::vec3 pos, ItemType type);

	ItemType type;
private:

	int id;
	float rotSpeed;
	float timer;

	Transform* model;
};

