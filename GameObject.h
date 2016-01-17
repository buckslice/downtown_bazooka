#pragma once

#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class GameObject {
// a base class for all of the objects that will be in the game
public:
	GameObject(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 rotation = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f));

	glm::vec3 getPosition();

	glm::vec3 getRotation();

	glm::vec3 getScale();

	void move(glm::vec3 newPosition);

	void rotate(glm::vec3 newRotation);

	void resize(glm::vec3 newScale);

private:
	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;
};