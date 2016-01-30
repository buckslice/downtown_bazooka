#include "noise.h"
#include <algorithm>
#include <math.h>

// The gradients are the midpoints of the vertices of a cube.
static const int grad3[12][3] = {
    { 1,1,0 },{ -1,1,0 },{ 1,-1,0 },{ -1,-1,0 },
    { 1,0,1 },{ -1,0,1 },{ 1,0,-1 },{ -1,0,-1 },
    { 0,1,1 },{ 0,-1,1 },{ 0,1,-1 },{ 0,-1,-1 }
};

// The gradients are the midpoints of the vertices of a hypercube.
static const int grad4[32][4] = {
    { 0,1,1,1 },{ 0,1,1,-1 },{ 0,1,-1,1 },{ 0,1,-1,-1 },
    { 0,-1,1,1 },{ 0,-1,1,-1 },{ 0,-1,-1,1 },{ 0,-1,-1,-1 },
    { 1,0,1,1 },{ 1,0,1,-1 },{ 1,0,-1,1 },{ 1,0,-1,-1 },
    { -1,0,1,1 },{ -1,0,1,-1 },{ -1,0,-1,1 },{ -1,0,-1,-1 },
    { 1,1,0,1 },{ 1,1,0,-1 },{ 1,-1,0,1 },{ 1,-1,0,-1 },
    { -1,1,0,1 },{ -1,1,0,-1 },{ -1,-1,0,1 },{ -1,-1,0,-1 },
    { 1,1,1,0 },{ 1,1,-1,0 },{ 1,-1,1,0 },{ 1,-1,-1,0 },
    { -1,1,1,0 },{ -1,1,-1,0 },{ -1,-1,1,0 },{ -1,-1,-1,0 }
};

// Permutation table.  The same list is repeated twice.
static const int perm[512] = {
    151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,
    8,99,37,240,21,10,23,190,6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,
    35,11,32,57,177,33,88,237,149,56,87,174,20,125,136,171,168,68,175,74,165,71,
    134,139,48,27,166,77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,
    55,46,245,40,244,102,143,54,65,25,63,161,1,216,80,73,209,76,132,187,208, 89,
    18,169,200,196,135,130,116,188,159,86,164,100,109,198,173,186,3,64,52,217,226,
    250,124,123,5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,
    189,28,42,223,183,170,213,119,248,152,2,44,154,163,70,221,153,101,155,167,43,
    172,9,129,22,39,253,19,98,108,110,79,113,224,232,178,185,112,104,218,246,97,
    228,251,34,242,193,238,210,144,12,191,179,162,241,81,51,145,235,249,14,239,
    107,49,192,214,31,181,199,106,157,184,84,204,176,115,121,50,45,127,4,150,254,
    138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180,

    151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,
    8,99,37,240,21,10,23,190,6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,
    35,11,32,57,177,33,88,237,149,56,87,174,20,125,136,171,168,68,175,74,165,71,
    134,139,48,27,166,77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,
    55,46,245,40,244,102,143,54,65,25,63,161,1,216,80,73,209,76,132,187,208, 89,
    18,169,200,196,135,130,116,188,159,86,164,100,109,198,173,186,3,64,52,217,226,
    250,124,123,5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,
    189,28,42,223,183,170,213,119,248,152,2,44,154,163,70,221,153,101,155,167,43,
    172,9,129,22,39,253,19,98,108,110,79,113,224,232,178,185,112,104,218,246,97,
    228,251,34,242,193,238,210,144,12,191,179,162,241,81,51,145,235,249,14,239,
    107,49,192,214,31,181,199,106,157,184,84,204,176,115,121,50,45,127,4,150,254,
    138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
};

// 2D Multi-octave Simplex noise.
// For each octave, a higher frequency/lower amplitude function will be added to the original.
// The higher the persistence [0-1], the more of each succeeding octave will be added.
float Noise::fractal_2D(float x, float y, float octaves, float scale, float persistence, float lacunarity) {
    float total = 0;
    float frequency = scale;
    float amplitude = 1;

    // We have to keep track of the largest possible amplitude,
    // because each octave adds more, and we need a value in [-1, 1].
    float maxAmplitude = 0;

    for (int i = 0; i < octaves; i++) {
        total += raw_2D(x * frequency, y * frequency) * amplitude;

        maxAmplitude += amplitude;
        amplitude *= persistence;
        frequency *= lacunarity;
    }

    return total / maxAmplitude;
}
// 3D Multi-octave Simplex noise.
// For each octave, a higher frequency/lower amplitude function will be added to the original.
// The higher the persistence [0-1], the more of each succeeding octave will be added.
float Noise::fractal_3D(float x, float y, float z, float octaves, float scale, float persistence, float lacunarity) {
    float total = 0;
    float frequency = scale;
    float amplitude = 1;

    // We have to keep track of the largest possible amplitude,
    // because each octave adds more, and we need a value in [-1, 1].
    float maxAmplitude = 0;

    for (int i = 0; i < octaves; i++) {
        total += raw_3D(x * frequency, y * frequency, z * frequency) * amplitude;

        maxAmplitude += amplitude;
        amplitude *= persistence;
        frequency *= lacunarity;
    }

    return total / maxAmplitude;
}


float Noise::ridged_2D(float x, float y, float octaves, float scale, float persistence, float lacunarity) {
    float total = 0;
    float frequency = scale;
    float amplitude = 1;

    for (int i = 0; i < octaves; i++) {
        float noise = raw_2D(x * frequency, y * frequency);
        total += (1.0f - fabsf(noise))*amplitude;
        amplitude *= persistence;
        frequency *= lacunarity;
    }

    return total - 1.0f;
}

// 2D Scaled Multi-octave Simplex noise.
// Returned value will be between loBound and hiBound.
float Noise::fractal_scaled_2D(float x, float y, float octaves, float scale, float low, float high, float persistence, float lacunarity) {
    return fractal_2D(x, y, octaves, scale, persistence, lacunarity) * (high - low) / 2 + (high + low) / 2;
}
// 3D Scaled Multi-octave Simplex noise.
// Returned value will be between loBound and hiBound.
float Noise::fractal_scaled_3D(float x, float y, float z, float octaves, float scale, float low, float high, float persistence, float lacunarity) {
    return fractal_3D(x, y, z, octaves, scale, persistence, lacunarity)  * (high - low) / 2 + (high + low) / 2;
}

// 2D Scaled Simplex raw noise.
// Returned value will be between loBound and hiBound.
float Noise::scaled_2D(float x, float y, float low, float high) {
    return raw_2D(x, y) * (high - low) / 2 + (high + low) / 2;
}
// 3D Scaled Simplex raw noise.
// Returned value will be between loBound and hiBound.
float Noise::scaled_3D(float x, float y, float z, float low, float high) {
    return raw_3D(x, y, z) * (high - low) / 2 + (high + low) / 2;
}

// 2D raw Simplex noise
float Noise::raw_2D(float x, float y) {
    // Noise contributions from the three corners
    float n0, n1, n2;

    // Skew the input space to determine which simplex cell we're in
    float F2 = 0.5 * (sqrtf(3.0) - 1.0);
    // Hairy factor for 2D
    float s = (x + y) * F2;
    int i = fastfloor(x + s);
    int j = fastfloor(y + s);

    float G2 = (3.0 - sqrtf(3.0)) / 6.0;
    float t = (i + j) * G2;
    // Unskew the cell origin back to (x,y) space
    float X0 = i - t;
    float Y0 = j - t;
    // The x,y distances from the cell origin
    float x0 = x - X0;
    float y0 = y - Y0;

    // For the 2D case, the simplex shape is an equilateral triangle.
    // Determine which simplex we are in.
    int i1, j1; // Offsets for second (middle) corner of simplex in (i,j) coords
    if (x0 > y0) { i1 = 1; j1 = 0; } // lower triangle, XY order: (0,0)->(1,0)->(1,1)
    else { i1 = 0; j1 = 1; } // upper triangle, YX order: (0,0)->(0,1)->(1,1)

                             // A step of (1,0) in (i,j) means a step of (1-c,-c) in (x,y), and
                             // a step of (0,1) in (i,j) means a step of (-c,1-c) in (x,y), where
                             // c = (3-sqrt(3))/6
    float x1 = x0 - i1 + G2; // Offsets for middle corner in (x,y) unskewed coords
    float y1 = y0 - j1 + G2;
    float x2 = x0 - 1.0 + 2.0 * G2; // Offsets for last corner in (x,y) unskewed coords
    float y2 = y0 - 1.0 + 2.0 * G2;

    // Work out the hashed gradient indices of the three simplex corners
    int ii = i & 255;
    int jj = j & 255;
    int gi0 = perm[ii + perm[jj]] % 12;
    int gi1 = perm[ii + i1 + perm[jj + j1]] % 12;
    int gi2 = perm[ii + 1 + perm[jj + 1]] % 12;

    // Calculate the contribution from the three corners
    float t0 = 0.5 - x0*x0 - y0*y0;
    if (t0 < 0) n0 = 0.0;
    else {
        t0 *= t0;
        n0 = t0 * t0 * dot(grad3[gi0], x0, y0); // (x,y) of grad3 used for 2D gradient
    }

    float t1 = 0.5 - x1*x1 - y1*y1;
    if (t1 < 0) n1 = 0.0;
    else {
        t1 *= t1;
        n1 = t1 * t1 * dot(grad3[gi1], x1, y1);
    }

    float t2 = 0.5 - x2*x2 - y2*y2;
    if (t2 < 0) n2 = 0.0;
    else {
        t2 *= t2;
        n2 = t2 * t2 * dot(grad3[gi2], x2, y2);
    }

    // Add contributions from each corner to get the final noise value.
    // The result is scaled to return values in the interval [-1,1].
    return 70.0 * (n0 + n1 + n2);
}

// 3D raw Simplex noise
float Noise::raw_3D(float x, float y, float z) {
    float n0, n1, n2, n3; // Noise contributions from the four corners

                          // Skew the input space to determine which simplex cell we're in
    float F3 = 1.0 / 3.0;
    float s = (x + y + z)*F3; // Very nice and simple skew factor for 3D
    int i = fastfloor(x + s);
    int j = fastfloor(y + s);
    int k = fastfloor(z + s);

    float G3 = 1.0 / 6.0; // Very nice and simple unskew factor, too
    float t = (i + j + k)*G3;
    float X0 = i - t; // Unskew the cell origin back to (x,y,z) space
    float Y0 = j - t;
    float Z0 = k - t;
    float x0 = x - X0; // The x,y,z distances from the cell origin
    float y0 = y - Y0;
    float z0 = z - Z0;

    // For the 3D case, the simplex shape is a slightly irregular tetrahedron.
    // Determine which simplex we are in.
    int i1, j1, k1; // Offsets for second corner of simplex in (i,j,k) coords
    int i2, j2, k2; // Offsets for third corner of simplex in (i,j,k) coords

    if (x0 >= y0) {
        if (y0 >= z0) { i1 = 1; j1 = 0; k1 = 0; i2 = 1; j2 = 1; k2 = 0; } // X Y Z order
        else if (x0 >= z0) { i1 = 1; j1 = 0; k1 = 0; i2 = 1; j2 = 0; k2 = 1; } // X Z Y order
        else { i1 = 0; j1 = 0; k1 = 1; i2 = 1; j2 = 0; k2 = 1; } // Z X Y order
    } else { // x0<y0
        if (y0 < z0) { i1 = 0; j1 = 0; k1 = 1; i2 = 0; j2 = 1; k2 = 1; } // Z Y X order
        else if (x0 < z0) { i1 = 0; j1 = 1; k1 = 0; i2 = 0; j2 = 1; k2 = 1; } // Y Z X order
        else { i1 = 0; j1 = 1; k1 = 0; i2 = 1; j2 = 1; k2 = 0; } // Y X Z order
    }

    // A step of (1,0,0) in (i,j,k) means a step of (1-c,-c,-c) in (x,y,z),
    // a step of (0,1,0) in (i,j,k) means a step of (-c,1-c,-c) in (x,y,z), and
    // a step of (0,0,1) in (i,j,k) means a step of (-c,-c,1-c) in (x,y,z), where
    // c = 1/6.
    float x1 = x0 - i1 + G3; // Offsets for second corner in (x,y,z) coords
    float y1 = y0 - j1 + G3;
    float z1 = z0 - k1 + G3;
    float x2 = x0 - i2 + 2.0*G3; // Offsets for third corner in (x,y,z) coords
    float y2 = y0 - j2 + 2.0*G3;
    float z2 = z0 - k2 + 2.0*G3;
    float x3 = x0 - 1.0 + 3.0*G3; // Offsets for last corner in (x,y,z) coords
    float y3 = y0 - 1.0 + 3.0*G3;
    float z3 = z0 - 1.0 + 3.0*G3;

    // Work out the hashed gradient indices of the four simplex corners
    int ii = i & 255;
    int jj = j & 255;
    int kk = k & 255;
    int gi0 = perm[ii + perm[jj + perm[kk]]] % 12;
    int gi1 = perm[ii + i1 + perm[jj + j1 + perm[kk + k1]]] % 12;
    int gi2 = perm[ii + i2 + perm[jj + j2 + perm[kk + k2]]] % 12;
    int gi3 = perm[ii + 1 + perm[jj + 1 + perm[kk + 1]]] % 12;

    // Calculate the contribution from the four corners
    float t0 = 0.6 - x0*x0 - y0*y0 - z0*z0;
    if (t0 < 0) n0 = 0.0;
    else {
        t0 *= t0;
        n0 = t0 * t0 * dot(grad3[gi0], x0, y0, z0);
    }

    float t1 = 0.6 - x1*x1 - y1*y1 - z1*z1;
    if (t1 < 0) n1 = 0.0;
    else {
        t1 *= t1;
        n1 = t1 * t1 * dot(grad3[gi1], x1, y1, z1);
    }

    float t2 = 0.6 - x2*x2 - y2*y2 - z2*z2;
    if (t2 < 0) n2 = 0.0;
    else {
        t2 *= t2;
        n2 = t2 * t2 * dot(grad3[gi2], x2, y2, z2);
    }

    float t3 = 0.6 - x3*x3 - y3*y3 - z3*z3;
    if (t3 < 0) n3 = 0.0;
    else {
        t3 *= t3;
        n3 = t3 * t3 * dot(grad3[gi3], x3, y3, z3);
    }

    // Add contributions from each corner to get the final noise value.
    // The result is scaled to stay just inside [-1,1]
    return 32.0*(n0 + n1 + n2 + n3);
}

int Noise::fastfloor(float x) { return x > 0 ? (int)x : (int)x - 1; }

float Noise::dot(const int* g, float x, float y) { return g[0] * x + g[1] * y; }
float Noise::dot(const int* g, float x, float y, float z) { return g[0] * x + g[1] * y + g[2] * z; }

// worley noise now


typedef unsigned int uint;
const uint OFFSET_BASIS = 2166136261;
const uint FNV_PRIME = 16777619;
const int seed = 3221;
static float darray[3];

float Noise::cf1(float ar[]) {
    return ar[0];
}
float Noise::cf2(float ar[]) {
    return ar[1] - ar[0];
}
float Noise::cf3(float ar[]) {
    return ar[2] - ar[0];
}

float Noise::euclidian(glm::vec3 a, glm::vec3 b) {
    return (a.x - b.x)*(a.x - b.x) + (a.y - b.y)*(a.y - b.y) + (a.z - b.z)*(a.z - b.z);
}
float Noise::manhattan(glm::vec3 a, glm::vec3 b) {
    return abs(a.x - b.x) + abs(a.y - b.y) + abs(a.z - b.z);
}
float Noise::chebyshev(glm::vec3 a, glm::vec3 b) {
    return std::max(std::max(abs(a.x - b.x), abs(a.y - b.y)), abs(a.z - b.z));
}

void Noise::insert(float ar[], float value) {
    float temp;

    int arraySize = sizeof(ar);

    for (int i = arraySize - 1; i >= 0; i--) {
        if (value > ar[i]) break;
        temp = ar[i];
        ar[i] = value;
        if (i + 1 < arraySize) ar[i + 1] = temp;
    }
}


uint Noise::prob(uint value) {
    if (value < 393325350) return 1;
    if (value < 1022645910) return 2;
    if (value < 1861739990) return 3;
    if (value < 2700834071) return 4;
    if (value < 3372109335) return 5;
    if (value < 3819626178) return 6;
    if (value < 4075350088) return 7;
    if (value < 4203212043) return 8;
    return 9;
}

uint Noise::lcgRandom(uint lastValue) {
    return (uint)((1103515245u * lastValue + 12345u) % 0x100000000u);
}

uint Noise::hash(uint i, uint j, uint k) {
    return (uint)((((((OFFSET_BASIS ^ (uint)i) * FNV_PRIME) ^ (uint)j) * FNV_PRIME) ^ (uint)k) * FNV_PRIME);
}

int Noise::hash(glm::vec3 input) {
    return (int)(input.x * 73856093) ^ (int)(input.y * 19349663) ^ (int)(input.z * 83492791);
}

float Noise::worley(glm::vec3 in) {
    float value = 0;
    uint lastRandom;
    uint numberFeaturePoints;
    glm::vec3 randomDiff = glm::vec3(0.0f);
    glm::vec3 featurePoint = glm::vec3(0.0f);
    int cubeX, cubeY, cubeZ;

    for (int i = 0; i < 3; ++i) {
        darray[i] = 9999999;
    }
    
    int evalCubeX = fastfloor(in.x);
    int evalCubeY = fastfloor(in.y);
    int evalCubeZ = fastfloor(in.z);

    for (int i = -1; i < 2; ++i) {
        for (int j = -1; j < 2; ++j) {
            for (int k = -1; k < 2; ++k) {
                cubeX = evalCubeX + i;
                cubeY = evalCubeY + j;
                cubeZ = evalCubeZ + k;

                lastRandom = lcgRandom(hash((uint)(cubeX + seed), (uint)(cubeY), (uint)(cubeZ)));

                numberFeaturePoints = prob(lastRandom);

                for (uint l = 0; l < numberFeaturePoints; ++l) {
                    lastRandom = lcgRandom(lastRandom);
                    randomDiff.x = (float)lastRandom / 0x100000000;

                    lastRandom = lcgRandom(lastRandom);
                    randomDiff.y = (float)lastRandom / 0x100000000;

                    lastRandom = lcgRandom(lastRandom);
                    randomDiff.z = (float)lastRandom / 0x100000000;

                    featurePoint.x = randomDiff.x + (float)cubeX;
                    featurePoint.y = randomDiff.y + (float)cubeY;
                    featurePoint.z = randomDiff.z + (float)cubeZ;

                    insert(darray, euclidian(in, featurePoint));
                }
            }
        }
    }

    // some weird shit is going on with the random number generator
    // or something. works but has random artifacts?
    // when you regenerate sometimes its just white or black ??? wtf blazzard
    // prob since im using statics or something and shits weird
    float ret = cf2(darray);
    if (ret < 0) {
        ret = 0;
    }
    if (ret > 1) {
        ret = 1;
    }
    return ret;
}