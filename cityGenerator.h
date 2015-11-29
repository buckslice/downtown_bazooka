#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include "mesh.h"
#include "aabb.h"
#include "graphics.h"

const float CITY_SIZE = 2000.0f;    // distance from one side to another
const float BLEND_DISTANCE = 500.0f;

class CityGenerator {
public:
    std::vector<glm::mat4> models;
    std::vector<glm::vec3> colors;
	std::vector<AABB> boxes;

	CityGenerator();

	void generate(bool square, GLuint count, Graphics& g);

private:
    bool first = true;
    GLuint colorBuffer;
    GLuint modelBuffer;

};