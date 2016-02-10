#pragma once

#include <stdlib.h>	//rand
#include <cmath>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <functional>
#include <algorithm>

extern const float PI;
extern const float RADSTODEGREES;
extern const float DEGREESTORADS;

class Mth {
public:

    // BIG TODO
    // Redo all this shit to use new random from c++11
    // its faster and more uniform!!!!!!!!!!!!!!!!!!
    static float rand01();

    static float rand0X(float X);

    static float randUnit();

    static float randRange(float a, float b);

    static glm::vec3 randInsideUnitCube();

    static glm::vec3 randInsideSphere(float radius);

    static glm::vec2 randomPointInSquare(float size);

    static glm::vec2 randomPointInCircle(float radius);

    static float clamp(float n, float min, float max);

    static int clamp(int n, int min, int max);

    static float saturate(float n);

    static float quadratic(float value);

    static float cubicSCurve(float value);

    static float cubic(float value);

    // returns a value from 0-1 that you can scale stuff by
    static float blend(float d, float low, float high, std::function<float(float)> f);

    // linear blend
    static float blend(float d, float low, float high);

    // clamped linear blend
    static float cblend(float d, float low, float high);

    static glm::vec3 lerp(const glm::vec3& a, const glm::vec3& b, float t);

};