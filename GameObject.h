#pragma once

#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class GameObject {
// a base class for all of the objects that will be in the game
public:
	GameObject(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 rotation = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f)) {
		this->position = position;
		this->rotation = rotation;
		this->scale = scale;
	}

	glm::vec3 getPosition() {
		return position;
	}

	glm::vec3 getRotation() {
		return rotation;
	}

	glm::vec3 getScale() {
		return scale;
	}

	void move(glm::vec3 newPosition) {
		position = newPosition;
	}

	void rotate(glm::vec3 newRotation) {
		rotation = newRotation;
	}

	void resize(glm::vec3 newScale) {
		scale = newScale;
	}

private:
	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;
};