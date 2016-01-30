#pragma once
#include <glm/glm.hpp>

// TODO: go through and rename scale to frequency for arguments
class Noise {
public:
    // Multi-octave Simplex noise
    // For each octave, a higher frequency/lower amplitude function will be added to the original.
    // The higher the persistence [0-1], the more of each succeeding octave will be added.
    static float fractal_2D(float x, float y, float octaves, float scale, float persistence = 0.5f, float lacunarity = 2.0f);
    static float fractal_3D(float x, float y, float z, float octaves, float scale, float persistence = 0.5f, float lacunarity = 2.0f);

    static float ridged_2D(float x, float y, float octaves, float scale, float persistence = 0.5f, float lacunarity = 2.0f);

    // Scaled Multi-octave Simplex noise
    // The result will be between the two parameters passed.
    static float fractal_scaled_2D(float x, float y, float octaves, float scale, float low, float high, float persistence = 0.5f, float lacunarity = 2.0f);
    static float fractal_scaled_3D(float x, float y, float z, float octaves, float scale, float low, float high, float persistence = 0.5f, float lacunarity = 2.0f);

    // Scaled Raw Simplex noise
    // The result will be between the two parameters passed.
    static float scaled_2D(float x, float y, float low, float high);
    static float scaled_3D(float x, float y, float z, float low, float high);

    // Raw Simplex noise - a single noise value. [-1,1]
    static float raw_2D(float x, float y);
    static float raw_3D(float x, float y, float z);

    // cellular noise
    static float worley(glm::vec3 in);

private:
    static int fastfloor(float x);

    static float dot(const int* g, float x, float y);
    static float dot(const int* g, float x, float y, float z);

    // worley stuff
    typedef unsigned int uint;

    static float cf1(float ar[]);
    static float cf2(float ar[]);
    static float cf3(float ar[]);

    static float euclidian(glm::vec3 p1, glm::vec3 p2);
    static float manhattan(glm::vec3 p1, glm::vec3 p2);
    static float chebyshev(glm::vec3 p1, glm::vec3 p2);

    static void insert(float arr[], float value);
    static uint prob(uint value);
    static uint lcgRandom(uint last);
    static uint hash(uint i, uint j, uint k);

    static int hash(glm::vec3 input);
};