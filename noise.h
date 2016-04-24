#pragma once
#include <glm/glm.hpp>

// JOHNS U7T1M4T3 NOISE LIBRARY #420BILBROBONGINS
// built and combined from many different sources found across the webs
class Noise {
public:
    // Multi-octave Simplex noise
    // For each octave, a higher frequency/lower amplitude function will be added to the original.
    // The higher the persistence (should be value from 0-1, the more of each succeeding octave will be added.
    // noise returned in range [-1,1]
    static float fractal_2D(float x, float y,
        float octaves, float frequency, float persistence = 0.5f, float lacunarity = 2.0f);
    static float fractal_3D(float x, float y, float z,
        float octaves, float frequency, float persistence = 0.5f, float lacunarity = 2.0f);

    static float ridged_2D(float x, float y,
        float octaves, float frequency, float persistence = 0.5f, float lacunarity = 2.0f);
    
    // Scaled Multi-octave Simplex noise
    // The result will be between the two parameters passed.
    static float fractal_scaled_2D(float x, float y,
        float octaves, float frequency, float low, float high, float persistence = 0.5f, float lacunarity = 2.0f);
    static float fractal_scaled_3D(float x, float y, float z,
        float octaves, float frequency, float low, float high, float persistence = 0.5f, float lacunarity = 2.0f);

    // Scaled Raw Simplex noise
    // The result will be between the two parameters passed.
    static float scaled_2D(float x, float y, float low, float high);
    static float scaled_3D(float x, float y, float z, float low, float high);

    // Raw Simplex noise - a single noise value. [-1,1]
    static float raw_2D(float x, float y);
    static float raw_3D(float x, float y, float z);

    // worley (cellular) noise
    // based off: https://github.com/bhickey/worley
    // TODO: convert to use C++11 numerics library
    enum DIST_FUNC {
        EUCLIDIAN,
        MANHATTAN,
        CHEBYSHEV
    };

    // (x,y,z) is input point
    // max_order specifies size of arrays
    // F and ID are arrays that get filled with requested features and ids
    // dfunc specifies which distance function to use
    // common combinations: F[0], F[1]-F[0], F[2]-F[0], F[0]*F[1], 1-F[x]
    // coloring can be based of IDs of each feature 
    // output range is from [0-1] (roughly)
    static void worley(float x, float y, float z, 
        size_t max_order, double* F, uint32_t* ID, DIST_FUNC dfunc, float frequency = 1.0f);

    // need to think about nice way to generate fractal since it has a more complicated return type
    // maybe just multiple different functions based on specified order? or just calculate all with array ptrs?

    static float fractal_worley_3D(float x, float y, float z, DIST_FUNC dfunc,
        float octaves, float frequency, float persistence = 0.5f, float lacunarity = 2.0f);

private:
    static int fastfloor(float x);

    static float dot(const int* g, float x, float y);
    static float dot(const int* g, float x, float y, float z);

    // worley
    static void addSamples(int32_t xi, int32_t yi, int32_t zi,
        size_t max_order, double* F, uint32_t* ID);
};