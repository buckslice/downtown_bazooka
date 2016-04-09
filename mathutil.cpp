#include "mathutil.h"

const float PI = 3.14159265358979f;
const float RADSTODEGREES = 180.0f / PI;
const float DEGREESTORADS = PI / 180.0f;

std::random_device rd;       // random device used to seed the generator
std::mt19937 Mth::rng(rd());

float Mth::rand01() {
    // RS: below function returns number [0,1) - very rarely returns 1.0 - (10 bits of randomness)
    return std::generate_canonical<float, 10>(rng);
}
float Mth::rand0X(float X) {
    std::uniform_real_distribution<> uni(0.0f, X);
    return static_cast <float> (uni(rng));
}
float Mth::randUnit() {
    std::uniform_real_distribution<float> uni(-1.0f, 1.0f);
    return uni(rng);
}
float Mth::randRange(float a, float b) {
    std::uniform_real_distribution<float> uni(a, b);
    return uni(rng);
}

glm::vec3 Mth::randInsideUnitCube() {
    return glm::vec3(randUnit(), randUnit(), randUnit());
}

glm::vec3 Mth::randInsideSphere(float radius) {
    float phi = rand0X(PI*2.0f);
    float costheta = randUnit();
    float u = rand01();

    float theta = acos(costheta);
    float r = radius * std::pow(u, 1.0f / 3.0f);
    return glm::vec3(sin(theta)*cos(phi), sin(theta)*sin(phi), cos(theta))*r;
}

glm::vec2 Mth::randomPointInSquare(float size) {
    return glm::vec2(rand01() * size - size / 2.0f, rand01() * size - size / 2.0f);
}

glm::vec2 Mth::randomPointInCircle(float radius) {
    float t = PI*2.0f* rand01();
    float u = rand01() + rand01();
    float r = u > 1 ? 2.0f - u : u;
    return glm::vec2(r * cos(t), r * sin(t)) * radius;
}

float Mth::clamp(float n, float min, float max) {
    return std::max(min, std::min(n, max));
}

int Mth::clamp(int n, int min, int max) {
    return std::min(std::max(n, min), max);
}

float Mth::saturate(float n) {
    return std::max(0.0f, std::min(n, 1.0f));
}

float Mth::quadratic(float x) {
    return x*x;
}
float Mth::cubic(float x) {
    return x*x*x;
}
float Mth::cubicSCurve(float x) {
    return x * x * (3.0f - 2.0f * x);
}
float Mth::smootherStep(float x) {
    return x*x*x*(x*(x * 6 - 15) + 10);
}

float Mth::blend(float x, float low, float high, std::function<float(float)> f) {
    return f((x - low) / (high - low));
}
float Mth::cblend(float x, float low, float high, std::function<float(float)> f) {
    return f(std::max(0.0f, std::min((x - low) / (high - low), 1.0f)));
}

float Mth::blend(float d, float low, float high) {
    return (d - low) / (high - low);
}
float Mth::cblend(float d, float low, float high) {
    return std::max(0.0f, std::min((d - low) / (high - low), 1.0f));
}

glm::vec3 Mth::lerp(const glm::vec3& a, const glm::vec3& b, float t) {
    return a + (b - a) * t;
}
