#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Entity {
public:
	Entity(glm::vec3 pos = glm::vec3(0.0f, 0.0f, 0.0f)) {
		this->pos = pos;
	}


private:
	glm::vec3 pos;

};