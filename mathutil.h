#pragma once

#include <stdlib.h>	//rand
#include <cmath>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <functional>
#include <algorithm>

class Mth {
public:
    static float rand01() {
        return static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    }

    static float rand0X(float X) {
        return static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / X));
    }

    static float randUnit() {
        return rand01() * 2.0f - 1.0f;
    }

    static glm::vec3 randInsideUnitSphere() {
        return glm::vec3(randUnit(), randUnit(), randUnit());
    }

    static glm::vec2 randomPointInSquare(float size) {
        return glm::vec2(rand01() * size - size / 2.0f, rand01() * size - size / 2.0f);
    }

    static glm::vec2 randomPointInCircle(float radius) {
        float t = 2 * 3.14159f * rand01();
        float u = rand01() + rand01();
        float r = u > 1 ? 2.0f - u : u;
        return glm::vec2(r * cos(t), r * sin(t)) * radius;
    }

    static float clamp(float n, float lower, float upper) {
        return std::max(lower, std::min(n, upper));
    }

    static float saturate(float n) {
        return std::max(0.0f, std::min(n, 1.0f));
    }

    static float cubicSCurve(float value) {
        return value * value * (3.0f - 2.0f * value);
    }

    static float cubic(float value) {
        return value*value*value;
    }

    static float blend(float d, float low, float high, std::function<float(float)> f) {
        return f((d - low) / (high - low));
    }

    static float gett(float d, float low, float high) {
        return (d - low) / (high - low);
    }

    static glm::vec3 lerp(const glm::vec3& a, const glm::vec3& b, float t) {
        return a + (b - a) * t;
    }

};