#pragma once

#include <cmath>
#include <vector>
#include <unordered_set>

#include "aabb.h"
#include "camera.h"
#include <SFML/Window.hpp>

class Physics {
public:
	const float GRAVITY = -9.8f;
    const float COL_RADIUS = 200.0f; // only checks for collisions in a radius around player

	// these should be moved into player class
	const GLfloat EYE_HEIGHT = 1.8f;
	const GLfloat P_HEIGHT = 2.0f;

	Physics(Camera& cam);

	void update(float delta);

	void addObject(AABB& obj);

	void addObjects(const std::vector<AABB>& objs);

	void clearObjects();

	// b1 is dynamic AABB with velocity vel
	// b2 is static AABB
	float sweepTest(AABB b1, AABB b2, glm::vec3 vel, glm::vec3& norm);

private:
	// eventually make collider class
	// so you can have different shapes
	std::vector<AABB> staticObjects;
	//std::vector<AABB> dynamicObjects;

    // holds address of each object already collided with this frame
    std::unordered_set<int> resolvedSet;

	Camera& cam;

	sf::Clock timeSinceStart;
};