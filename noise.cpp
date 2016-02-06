#include "noise.h"
#include <algorithm>
#include <math.h>
#include "mathutil.h"

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
float Noise::fractal_2D(float x, float y, float octaves, float frequency, float persistence, float lacunarity) {
    float total = 0;
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
float Noise::fractal_3D(float x, float y, float z, float octaves, float frequency, float persistence, float lacunarity) {
    float total = 0;
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


float Noise::ridged_2D(float x, float y, float octaves, float frequency, float persistence, float lacunarity) {
    float total = 0;
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
float Noise::fractal_scaled_2D(float x, float y,
    float octaves, float frequency, float low, float high, float persistence, float lacunarity) {
    return fractal_2D(x, y, octaves, frequency, persistence, lacunarity) * (high - low) / 2 + (high + low) / 2;
}
// 3D Scaled Multi-octave Simplex noise.
// Returned value will be between loBound and hiBound.
float Noise::fractal_scaled_3D(float x, float y, float z,
    float octaves, float frequency, float low, float high, float persistence, float lacunarity) {
    return fractal_3D(x, y, z, octaves, frequency, persistence, lacunarity)  * (high - low) / 2 + (high + low) / 2;
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




////////////////////////
///// WORLEY NOISE /////
////////////////////////

static int poisson[256] =
{ 4,3,1,1,1,2,4,2,2,2,5,1,0,2,1,2,2,0,4,3,2,1,2,1,3,2,2,4,2,2,5,1,2,3,2,2,2,2,2,3,
  2,4,2,5,3,2,2,2,5,3,3,5,2,1,3,3,4,4,2,3,0,4,2,2,2,1,3,2,2,2,3,3,3,1,2,0,2,1,1,2,
  2,2,2,5,3,2,3,2,3,2,2,1,0,2,1,1,2,1,2,2,1,3,4,2,2,2,5,4,2,4,2,2,5,4,3,2,2,5,4,3,
  3,3,5,2,2,2,2,2,3,1,1,4,2,1,3,3,4,3,2,4,3,3,3,4,5,1,4,2,4,3,1,2,3,5,3,2,1,3,1,3,
  3,3,2,3,1,5,5,4,2,2,4,1,3,4,1,5,3,3,5,3,4,3,2,2,1,1,1,1,1,2,4,5,4,5,4,2,1,5,1,1,
  2,3,3,3,2,5,2,3,3,2,0,2,1,1,4,2,1,3,2,1,2,2,3,2,5,5,3,4,5,5,2,4,4,5,3,2,2,2,1,4,
  2,3,3,4,2,5,4,2,4,2,2,2,4,5,3,2 };

inline int32_t dfloor(double x) {
    return x < 0 ? ((int32_t)x - 1) : ((int32_t)x);
}

inline double euclidian(double dx, double dy, double dz) {
    return dx*dx + dy*dy + dz*dz;
}
inline double manhattan(double dx, double dy, double dz) {
    return fabs(dx) + fabs(dy) + fabs(dz);
}
inline double chebyshev(double dx, double dy, double dz) {
    return std::max(std::max(fabs(dx), fabs(dy)), fabs(dz));
}

// private array for computation
static double at[3];
// pointer to distance function
double(*distance_function)(double, double, double);
// used to make sure mean value of F[0] is 1.0
// easy way to scale cellular features
//const double DENSITY_ADJUSTMENT = 0.398150;
const double DENSITY_ADJUSTMENT = 0.85;
//const double DENSITY_ADJUSTMENT = 1.0;

void Noise::worley(float x, float y, float z,
    size_t max_order, double* F, uint32_t* ID, DIST_FUNC dfunc, float frequency) {
    if (dfunc == EUCLIDIAN) {
        distance_function = &euclidian;
    } else if (dfunc == MANHATTAN) {
        distance_function = &manhattan;
    } else {
        distance_function = &chebyshev;
    }

    at[0] = x * frequency * DENSITY_ADJUSTMENT;
    at[1] = y * frequency * DENSITY_ADJUSTMENT;
    at[2] = z * frequency * DENSITY_ADJUSTMENT;

    double x2, y2, z2, mx2, my2, mz2;
    int32_t int_at[3];
    size_t i;

    for (i = 0; i < max_order; ++i) {
        F[i] = 999999.9;
    }

    int_at[0] = dfloor(at[0]);
    int_at[1] = dfloor(at[1]);
    int_at[2] = dfloor(at[2]);

    addSamples(int_at[0], int_at[1], int_at[2], max_order, F, ID);
    x2 = at[0] - int_at[0];
    y2 = at[1] - int_at[1];
    z2 = at[2] - int_at[2];
    mx2 = (1.0 - x2)*(1.0 - x2);
    my2 = (1.0 - y2)*(1.0 - y2);
    mz2 = (1.0 - z2)*(1.0 - z2);
    x2 *= x2;
    y2 *= y2;
    z2 *= z2;

    // 6 facing neighbors of center cube are closest
    // so they have greatest chance for feature point
    if (x2 < F[max_order - 1])  addSamples(int_at[0] - 1, int_at[1], int_at[2], max_order, F, ID);
    if (y2 < F[max_order - 1])  addSamples(int_at[0], int_at[1] - 1, int_at[2], max_order, F, ID);
    if (z2 < F[max_order - 1])  addSamples(int_at[0], int_at[1], int_at[2] - 1, max_order, F, ID);
    if (mx2 < F[max_order - 1]) addSamples(int_at[0] + 1, int_at[1], int_at[2], max_order, F, ID);
    if (my2 < F[max_order - 1]) addSamples(int_at[0], int_at[1] + 1, int_at[2], max_order, F, ID);
    if (mz2 < F[max_order - 1]) addSamples(int_at[0], int_at[1], int_at[2] + 1, max_order, F, ID);

    // next closest is 12 edge cubes
    if (x2 + y2 < F[max_order - 1]) addSamples(int_at[0] - 1, int_at[1] - 1, int_at[2], max_order, F, ID);
    if (x2 + z2 < F[max_order - 1]) addSamples(int_at[0] - 1, int_at[1], int_at[2] - 1, max_order, F, ID);
    if (y2 + z2 < F[max_order - 1]) addSamples(int_at[0], int_at[1] - 1, int_at[2] - 1, max_order, F, ID);
    if (mx2 + my2 < F[max_order - 1]) addSamples(int_at[0] + 1, int_at[1] + 1, int_at[2], max_order, F, ID);
    if (mx2 + mz2 < F[max_order - 1]) addSamples(int_at[0] + 1, int_at[1], int_at[2] + 1, max_order, F, ID);
    if (my2 + mz2 < F[max_order - 1]) addSamples(int_at[0], int_at[1] + 1, int_at[2] + 1, max_order, F, ID);
    if (x2 + my2 < F[max_order - 1]) addSamples(int_at[0] - 1, int_at[1] + 1, int_at[2], max_order, F, ID);
    if (x2 + mz2 < F[max_order - 1]) addSamples(int_at[0] - 1, int_at[1], int_at[2] + 1, max_order, F, ID);
    if (y2 + mz2 < F[max_order - 1]) addSamples(int_at[0], int_at[1] - 1, int_at[2] + 1, max_order, F, ID);
    if (mx2 + y2 < F[max_order - 1]) addSamples(int_at[0] + 1, int_at[1] - 1, int_at[2], max_order, F, ID);
    if (mx2 + z2 < F[max_order - 1]) addSamples(int_at[0] + 1, int_at[1], int_at[2] - 1, max_order, F, ID);
    if (my2 + z2 < F[max_order - 1]) addSamples(int_at[0], int_at[1] + 1, int_at[2] - 1, max_order, F, ID);

    // final 8 corners
    if (x2 + y2 + z2 < F[max_order - 1]) addSamples(int_at[0] - 1, int_at[1] - 1, int_at[2] - 1, max_order, F, ID);
    if (x2 + y2 + mz2 < F[max_order - 1]) addSamples(int_at[0] - 1, int_at[1] - 1, int_at[2] + 1, max_order, F, ID);
    if (x2 + my2 + z2 < F[max_order - 1]) addSamples(int_at[0] - 1, int_at[1] + 1, int_at[2] - 1, max_order, F, ID);
    if (x2 + my2 + mz2 < F[max_order - 1]) addSamples(int_at[0] - 1, int_at[1] + 1, int_at[2] + 1, max_order, F, ID);
    if (mx2 + y2 + z2 < F[max_order - 1]) addSamples(int_at[0] + 1, int_at[1] - 1, int_at[2] - 1, max_order, F, ID);
    if (mx2 + y2 + mz2 < F[max_order - 1]) addSamples(int_at[0] + 1, int_at[1] - 1, int_at[2] + 1, max_order, F, ID);
    if (mx2 + my2 + z2 < F[max_order - 1]) addSamples(int_at[0] + 1, int_at[1] + 1, int_at[2] - 1, max_order, F, ID);
    if (mx2 + my2 + mz2 < F[max_order - 1]) addSamples(int_at[0] + 1, int_at[1] + 1, int_at[2] + 1, max_order, F, ID);

    // We're done! Convert to right size scale
    for (i = 0; i < max_order; i++) {
        F[i] = sqrt(F[i]) * (1.0 / DENSITY_ADJUSTMENT);
    }
}

void Noise::addSamples(int32_t xi, int32_t yi, int32_t zi,
    size_t max_order, double* F, uint32_t* ID) {

    double dx, dy, dz, fx, fy, fz, d2;
    int32_t count, index;
    uint32_t seed;
    uint32_t this_id;

    // each cube has random number seed based on cubes ID
    // LCG using Knuth constants for maximal periods
    seed = 702395077 * xi + 915488749 * yi + 2120969693 * zi;
    count = poisson[seed >> 24];    // 256 element table lookup. using MSB
    seed = 1402024253 * seed + 586950981;   //churns seed

    for (int32_t j = 0; j < count; ++j) {
        this_id = seed;

        // get random values for fx,fy,fz based on seed
        seed = 1402024253 * seed + 586950981;
        fx = (seed + 0.5)*(1.0 / 4294967296.0);
        seed = 1402024253 * seed + 586950981;
        fy = (seed + 0.5)*(1.0 / 4294967296.0);
        seed = 1402024253 * seed + 586950981;
        fz = (seed + 0.5)*(1.0 / 4294967296.0);
        seed = 1402024253 * seed + 586950981;

        dx = xi + fx - at[0];
        dy = yi + fy - at[1];
        dz = zi + fz - at[2];

        // get distance squared using specified function
        d2 = distance_function(dx, dy, dz);

        // in order insertion
        if (d2 < F[max_order - 1]) {
            index = max_order;
            while (index > 0 && d2 < F[index - 1]) index--;

            // insert this new point into slot # <index>
            // bump down more distant information to make room for this new point.
            for (int32_t i = max_order - 2; i >= index; i--) {
                F[i + 1] = F[i];
                ID[i + 1] = ID[i];
            }
            // insert the new point's information into the list.
            F[index] = d2;
            ID[index] = this_id;
        }
    }
}

float Noise::fractal_worley_3D(float x, float y, float z, DIST_FUNC dfunc,
    float octaves, float frequency, float persistence, float lacunarity) {

    double total = 0;
    double amplitude = 1;

    float maxAmplitude = 0;

    const int max = 3;
    double f[max];
    uint32_t id[max];
    for (int i = 0; i < octaves; i++) {
        worley(x, y, z, max, f, id, dfunc, frequency);
        float n = static_cast<float>(f[1]-f[0]);
        total += n * amplitude;

        maxAmplitude += amplitude;
        amplitude *= persistence;
        frequency *= lacunarity;
    }

    return total / maxAmplitude;
}