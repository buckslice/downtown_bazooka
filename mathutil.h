#pragma once

#include <stdlib.h>	//rand
#include <cmath>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <functional>
#include <algorithm>

// theres are defined in the cpp file
extern const float PI;
extern const float RADSTODEGREES;   // multiply by this to convert radians to degrees
extern const float DEGREESTORADS;   // multiply by this to convert degrees to radians

class Mth {
public:

    // returns random float between 0 and 1
    static float rand01();

    // returns random float between 0 and X
    static float rand0X(float X);

    // returns random float between -1 and 1
    static float randUnit();

    // returns random float between a and b
    static float randRange(float a, float b);

    // returns random vector3 with each value between -1 and 1
    static glm::vec3 randInsideUnitCube();

    // returns random vector3 somewhere inside a unit sphere
    static glm::vec3 randInsideSphere(float radius);

    // returns random vector2 with each value between -size and size
    static glm::vec2 randomPointInSquare(float size);

    // returns random vector2 somewhere inside a circle with given radius
    static glm::vec2 randomPointInCircle(float radius);

    // returns n clamped between min and max
    // could make clamp use a template instead:
    // template<typename T>
    // static T clamp(T t, T min, T max);
    static float clamp(float n, float min, float max);
    static int clamp(int n, int min, int max);

    // clamps value between 0 and 1
    static float saturate(float n);

    // returns value^2
    static float quadratic(float value);

    // https://en.wikipedia.org/wiki/Sigmoid_function
    static float cubicSCurve(float value);

    // returns value^3
    static float cubic(float value);

    // returns a value from 0-1 that you can scale stuff by
    static float blend(float d, float low, float high, std::function<float(float)> f);

    // linear blend
    static float blend(float d, float low, float high);

    // clamped linear blend
    static float cblend(float d, float low, float high);

    // linear interpolates between two vectors
    static glm::vec3 lerp(const glm::vec3& a, const glm::vec3& b, float t);

};