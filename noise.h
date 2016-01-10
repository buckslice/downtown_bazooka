#pragma once

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

private:
    static int fastfloor(float x);

    static float dot(const int* g, float x, float y);
    static float dot(const int* g, float x, float y, float z);

};
