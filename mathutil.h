#pragma once

#include <stdlib.h>	//rand
#include <cmath>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <functional>
#include <algorithm>
#include <random>

// theres are defined in the cpp file
extern const float PI;
extern const float RADSTODEGREES;   // multiply by this to convert radians to degrees
extern const float DEGREESTORADS;   // multiply by this to convert degrees to radians

class Mth {
private:
    static std::mt19937 rng;        // generator uses a random device
    static std::mt19937 rngVector;
public:
    // returns random float [0, 1)
    static float rand01();

    // returns random float between -1 and 1 [-1, 1)
    static float randUnit();

    // returns random float [a, b)
    static float randRange(float a, float b);

    // returns random int [a, b)
    static int randRange(int a, int b);

    // returns random vec3 with each value between -1 and 1
    static glm::vec3 randInsideUnitCube();

    // returns random vec3 somewhere inside a unit sphere
    static glm::vec3 randInsideSphere(float radius);

    // return random vec3 on unit sphere
    static glm::vec3 randOnUnitSphere();

    // returns random vec3 with values between min and max
    static glm::vec3 randomPointInRange(glm::vec3 min, glm::vec3 max);

    // returns random vec2 with each value between -size and size
    static glm::vec2 randomPointInSquare(float size);

    // returns random vec2 somewhere inside a circle with given radius
    static glm::vec2 randomPointInCircle(float radius);

    // calculates a model matrix
    static glm::mat4 getModelMatrix(
        glm::vec3 pos = glm::vec3(),
        glm::vec3 scale = glm::vec3(),
        glm::quat rot = glm::quat());

    // returns n clamped between min and max
    // TODO make clamp use a template instead:
    // template<typename T>
    // static T clamp(T t, T min, T max);
    static float clamp(float n, float min, float max);
    static int clamp(int n, int min, int max);

    // clamps value between 0 and 1
    static float clamp01(float n);
    // clamps value between 0 and 1
    static float saturate(float n);

    // BLENDING FUNCTIONS
    // returns value^2
    static float quadratic(float x);
    // returns value^3
    static float cubic(float x);
    // look up "sigmoid function" or "smoothstep" on wikipedia for more info for these two
    static float cubicSCurve(float x);  // aka smoothstep
    static float smootherStep(float x);

    // blend and clamped blend functions
    // if x < low   returns 0.0f
    // if x > high  returns 1.0f;
    // otherwise returns a blend based on the function provided
    static float blend(float x, float low, float high, std::function<float(float)> f);
    static float cblend(float x, float low, float high, std::function<float(float)> f);
    // linear versions (no need for blend functions)
    static float blend(float x, float low, float high);
    static float cblend(float x, float low, float high);

    // linear interpolates between two vectors
    static glm::vec3 lerp(const glm::vec3& a, const glm::vec3& b, float t);

};