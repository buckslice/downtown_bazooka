#pragma once

#include <stdlib.h>	//rand
#include <cmath>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <functional>
#include <algorithm>

const float PI = 3.14159265358979f;
class Mth {
public:
    // BIG TODO
    // Redo all this shit to use new random from c++11
    // its faster and more uniform!!!!!!!!!!!!!!!!!!
    static float rand01() {
        return static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    }
    static float rand0X(float X) {
        return static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / X));
    }
    static float randUnit() {
        return static_cast <float> (rand()) / static_cast <float> (RAND_MAX) * 2.0f - 1.0f;
    }
    static float randRange(float a, float b) {
        if (a < b) {
            return rand0X(b - a) + a;
        }
        return rand0X(a - b) + b;
    }

    static glm::vec3 randInsideUnitCube() {
        return glm::vec3(randUnit(), randUnit(), randUnit());
    }

    static glm::vec3 randInsideSphere(float radius) {
        float phi = rand0X(PI*2.0f);
        float costheta = randUnit();
        float u = rand01();

        float theta = acos(costheta);
        float r = radius * std::pow(u, 1.0f / 3.0f);
        return glm::vec3(sin(theta)*cos(phi), sin(theta)*sin(phi), cos(theta))*r;
    }

    static glm::vec2 randomPointInSquare(float size) {
        return glm::vec2(rand01() * size - size / 2.0f, rand01() * size - size / 2.0f);
    }

    static glm::vec2 randomPointInCircle(float radius) {
        float t = PI*2.0f* rand01();
        float u = rand01() + rand01();
        float r = u > 1 ? 2.0f - u : u;
        return glm::vec2(r * cos(t), r * sin(t)) * radius;
    }

    static float clamp(float n, float min, float max) {
        return std::max(min, std::min(n, max));
    }

    static int clamp(int n, int min, int max) {
        return std::min(std::max(n, min), max);
    }

    static float saturate(float n) {
        return std::max(0.0f, std::min(n, 1.0f));
    }

    static float quadratic(float value) {
        return value*value;
    }

    static float cubicSCurve(float value) {
        return value * value * (3.0f - 2.0f * value);
    }

    static float cubic(float value) {
        return value*value*value;
    }

    // returns a value from 0-1 that you can scale stuff by
    static float blend(float d, float low, float high, std::function<float(float)> f) {
        return f((d - low) / (high - low));
    }

    // linear blend
    static float blend(float d, float low, float high) {
        return (d - low) / (high - low);
    }

    // clamped linear blend
    static float cblend(float d, float low, float high) {
        return std::max(0.0f, std::min((d - low) / (high - low), 1.0f));
    }

    static glm::vec3 lerp(const glm::vec3& a, const glm::vec3& b, float t) {
        return a + (b - a) * t;
    }

};