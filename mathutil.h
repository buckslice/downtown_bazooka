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

	static glm::vec2 randomPointInSquare(float size) {
		return glm::vec2(randFloat() * size - size / 2.0f, randFloat() * size - size / 2.0f);
	}

	static glm::vec2 randomPointInCircle(float radius) {
		float t = 2 * 3.14159f * randFloat();
		float u = randFloat() + randFloat();
		float r = u > 1 ? 2.0f - u : u;
		return glm::vec2(r * cos(t), r * sin(t)) * radius;
	}

	static float cubicSCurve(float value) {
		return (3.0f - 2.0f * value);
	}

	static float cubic(float value) {
		return value*value*value;
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