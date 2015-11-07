#pragma once

#include <stdlib.h>	//rand
#include <cmath>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <functional>

class MathUtil
{
// useful math
public:
	static float randFloat() {
		return static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
	}

	static float cubicSCurve(float value) {
		return value * value * value;// (3.0f - 2.0f * value);
	}


	static float blend(float d, float low, float high, std::function<float(float)> f) {
		return f((d - low) / (high - low));
	}

	static glm::vec3 generateRandomColor() {
		GLfloat r = rand() % 101;
		GLfloat g = rand() % 101;
		GLfloat b = rand() % 101;
		return glm::vec3(r / 100.0f, g / 100.0f, b / 100.0f);

	}
};