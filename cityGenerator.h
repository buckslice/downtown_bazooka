#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include "mesh.h"
#include "aabb.h"

const GLuint NUMBER_OF_MESHES = 7500;
const GLuint NUMBER_OF_VERTICES = 120;
const GLuint FLOATS_PER_VERTEX = 5;
const float CITY_SIZE = 2000.0f;
const float BLEND_DISTANCE = 500.0f;

class CityGenerator {
public:
	glm::mat4* modelMatrices = new glm::mat4[NUMBER_OF_MESHES];
	glm::vec3* colors = new glm::vec3[NUMBER_OF_MESHES];
	std::vector<AABB> boxes;

	CityGenerator();

	void generateModelMatrices(bool square);

	void uploadModelMatrices(Mesh& mesh);

	void destroy();

	Mesh buildMesh(GLuint& tex);

private:


};