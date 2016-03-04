#include "mathutil.h"

const float PI = 3.14159265358979f;
const float RADSTODEGREES = 180.0f / PI;
const float DEGREESTORADS = PI / 180.0f;
const float ZERO = 0.0f;
std::random_device rd; // RS: type says it all, a random device used for the generator
std::mt19937 gen(rd()); //RS: generator uses a random device



// BIG TODO
// Redo all this shit to use new random from c++11
// its faster and more uniform!!!!!!!!!!!!!!!!!!
float Mth::rand01() {
	// RS: below function returns number [0,1) - very rarely returns 1.0 - (10 bits of randomness)
	return static_cast <float> (std::generate_canonical<double, 10>(gen)); 
}
float Mth::rand0X(float X) {
	// RS: I have no idea how/why this works. BUT IT'S SUPPOSED TO
	std::uniform_real_distribution<> dis(ZERO, X);
    return static_cast <float> (dis(gen));
}
float Mth::randUnit() {
	// RS: Bad practice of hardcoding values - it's k cause this wil never change
	std::uniform_real_distribution<> dis(-1.0f, 1.0f);
	return static_cast <float> (dis(gen));
}
float Mth::randRange(float a, float b) {
    if (a < b) {
        return rand0X(b - a) + a;
    }
    return rand0X(a - b) + b;
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

float Mth::quadratic(float value) {
    return value*value;
}

float Mth::cubicSCurve(float value) {
    return value * value * (3.0f - 2.0f * value);
}

float Mth::cubic(float value) {
    return value*value*value;
}

// returns a value from 0-1 that you can scale stuff by
float Mth::blend(float d, float low, float high, std::function<float(float)> f) {
    return f((d - low) / (high - low));
}

// linear blend
float Mth::blend(float d, float low, float high) {
    return (d - low) / (high - low);
}

// clamped linear blend
float Mth::cblend(float d, float low, float high) {
    return std::max(0.0f, std::min((d - low) / (high - low), 1.0f));
}

glm::vec3 Mth::lerp(const glm::vec3& a, const glm::vec3& b, float t) {
    return a + (b - a) * t;
}
