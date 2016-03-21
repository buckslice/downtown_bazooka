#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include "mesh.h"
#include "aabb.h"
#include "physics.h"

const int MAX_TRIES = 100;
const float CITY_SIZE = 2000.0f;    // distance from one side to another
const float BLEND_DISTANCE = 500.0f;
const float MIN_DIST = 400.0f;
const float LOW_HEIGHT = 20.0f;
const float HIGH_HEIGHT = 60.0f;

class CityGenerator {
public:
    std::vector<glm::mat4> models;
    std::vector<glm::vec3> colors;

	CityGenerator();
    ~CityGenerator();

    void render();

    void generate(bool square, bool colorByAngle, GLuint count, Physics& phys);

private:
    TIMesh* buildingMesh;
};