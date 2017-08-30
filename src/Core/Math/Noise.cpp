// Copyright (c) 2008, Casey Duncan (casey dot duncan at gmail dot com)
#include "CoreMinimal.h"
#include "Noise.h"
#include <math.h>

#define lerp(t, a, b) ((a) + (t) * ((b) - (a)))
#define fastfloor(n) (int)(n) - (((n) < 0.0f) & ((n) != (int)(n)))
#if K3DCOMPILER_MSVC
#pragma warning(disable : 4244)
#endif
// Fast sine/cosine functions from
// http://devmaster.net/forums/topic/4648-fast-and-accurate-sinecosine/page__st__80
// Note the input to these functions is not radians
// instead x = [0, 2] for r = [0, 2*PI]

inline float fast_sin(float x)
{
  // Convert the input value to a range of -1 to 1
  // x = x * (1.0f / PI);

  // Wrap around
  volatile float z = (x + 25165824.0f);
  x = x - (z - 25165824.0f);

#if LOW_SINE_PRECISION
  return 4.0f * (x - x * fabsf(x));
#else
  {
    float y = x - x * fabsf(x);
    const float Q = 3.1f;
    const float P = 3.6f;
    return y * (Q + P * fabsf(y));
  }
#endif
}

inline float fast_cos(float x)
{
  return fast_sin(x + 0.5f);
}

namespace Perlin
{

// From https://github.com/caseman/noise/blob/master/_noise.h
const float GRAD3[][3] = {
  { 1,1,0 },{ -1,1,0 },{ 1,-1,0 },{ -1,-1,0 },
  { 1,0,1 },{ -1,0,1 },{ 1,0,-1 },{ -1,0,-1 },
  { 0,1,1 },{ 0,-1,1 },{ 0,1,-1 },{ 0,-1,-1 },
  { 1,0,-1 },{ -1,0,-1 },{ 0,-1,1 },{ 0,1,1 } };

const float GRAD4[][4] = {
  { 0,1,1,1 },{ 0,1,1,-1 },{ 0,1,-1,1 },{ 0,1,-1,-1 },
  { 0,-1,1,1 },{ 0,-1,1,-1 },{ 0,-1,-1,1 },{ 0,-1,-1,-1 },
  { 1,0,1,1 },{ 1,0,1,-1 },{ 1,0,-1,1 },{ 1,0,-1,-1 },
  { -1,0,1,1 },{ -1,0,1,-1 },{ -1,0,-1,1 },{ -1,0,-1,-1 },
  { 1,1,0,1 },{ 1,1,0,-1 },{ 1,-1,0,1 },{ 1,-1,0,-1 },
  { -1,1,0,1 },{ -1,1,0,-1 },{ -1,-1,0,1 },{ -1,-1,0,-1 },
  { 1,1,1,0 },{ 1,1,-1,0 },{ 1,-1,1,0 },{ 1,-1,-1,0 },
  { -1,1,1,0 },{ -1,1,-1,0 },{ -1,-1,1,0 },{ -1,-1,-1,0 } };

// At the possible cost of unaligned access, we use char instead of
// int here to try to ensure that this table fits in L1 cache
const unsigned char PERM[] = {
  151, 160, 137, 91, 90, 15, 131, 13, 201, 95, 96, 53, 194, 233, 7, 225, 140,
  36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23, 190, 6, 148, 247, 120,
  234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32, 57, 177, 33,
  88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175, 74, 165, 71,
  134, 139, 48, 27, 166, 77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133,
  230, 220, 105, 92, 41, 55, 46, 245, 40, 244, 102, 143, 54, 65, 25, 63, 161,
  1, 216, 80, 73, 209, 76, 132, 187, 208, 89, 18, 169, 200, 196, 135, 130,
  116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64, 52, 217, 226, 250,
  124, 123, 5, 202, 38, 147, 118, 126, 255, 82, 85, 212, 207, 206, 59, 227,
  47, 16, 58, 17, 182, 189, 28, 42, 223, 183, 170, 213, 119, 248, 152, 2, 44,
  154, 163, 70, 221, 153, 101, 155, 167, 43, 172, 9, 129, 22, 39, 253, 19, 98,
  108, 110, 79, 113, 224, 232, 178, 185, 112, 104, 218, 246, 97, 228, 251, 34,
  242, 193, 238, 210, 144, 12, 191, 179, 162, 241, 81, 51, 145, 235, 249, 14,
  239, 107, 49, 192, 214, 31, 181, 199, 106, 157, 184, 84, 204, 176, 115, 121,
  50, 45, 127, 4, 150, 254, 138, 236, 205, 93, 222, 114, 67, 29, 24, 72, 243,
  141, 128, 195, 78, 66, 215, 61, 156, 180, 151, 160, 137, 91, 90, 15, 131,
  13, 201, 95, 96, 53, 194, 233, 7, 225, 140, 36, 103, 30, 69, 142, 8, 99, 37,
  240, 21, 10, 23, 190, 6, 148, 247, 120, 234, 75, 0, 26, 197, 62, 94, 252,
  219, 203, 117, 35, 11, 32, 57, 177, 33, 88, 237, 149, 56, 87, 174, 20, 125,
  136, 171, 168, 68, 175, 74, 165, 71, 134, 139, 48, 27, 166, 77, 146, 158,
  231, 83, 111, 229, 122, 60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245,
  40, 244, 102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187,
  208, 89, 18, 169, 200, 196, 135, 130, 116, 188, 159, 86, 164, 100, 109, 198,
  173, 186, 3, 64, 52, 217, 226, 250, 124, 123, 5, 202, 38, 147, 118, 126,
  255, 82, 85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182, 189, 28, 42, 223,
  183, 170, 213, 119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167,
  43, 172, 9, 129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185,
  112, 104, 218, 246, 97, 228, 251, 34, 242, 193, 238, 210, 144, 12, 191, 179,
  162, 241, 81, 51, 145, 235, 249, 14, 239, 107, 49, 192, 214, 31, 181, 199,
  106, 157, 184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254, 138, 236,
  205, 93, 222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156,
  180 };

const unsigned char SIMPLEX[][4] = {
  { 0,1,2,3 },{ 0,1,3,2 },{ 0,0,0,0 },{ 0,2,3,1 },{ 0,0,0,0 },{ 0,0,0,0 },{ 0,0,0,0 },
  { 1,2,3,0 },{ 0,2,1,3 },{ 0,0,0,0 },{ 0,3,1,2 },{ 0,3,2,1 },{ 0,0,0,0 },{ 0,0,0,0 },
  { 0,0,0,0 },{ 1,3,2,0 },{ 0,0,0,0 },{ 0,0,0,0 },{ 0,0,0,0 },{ 0,0,0,0 },{ 0,0,0,0 },
  { 0,0,0,0 },{ 0,0,0,0 },{ 0,0,0,0 },{ 1,2,0,3 },{ 0,0,0,0 },{ 1,3,0,2 },{ 0,0,0,0 },
  { 0,0,0,0 },{ 0,0,0,0 },{ 2,3,0,1 },{ 2,3,1,0 },{ 1,0,2,3 },{ 1,0,3,2 },{ 0,0,0,0 },
  { 0,0,0,0 },{ 0,0,0,0 },{ 2,0,3,1 },{ 0,0,0,0 },{ 2,1,3,0 },{ 0,0,0,0 },{ 0,0,0,0 },
  { 0,0,0,0 },{ 0,0,0,0 },{ 0,0,0,0 },{ 0,0,0,0 },{ 0,0,0,0 },{ 0,0,0,0 },{ 2,0,1,3 },
  { 0,0,0,0 },{ 0,0,0,0 },{ 0,0,0,0 },{ 3,0,1,2 },{ 3,0,2,1 },{ 0,0,0,0 },{ 3,1,2,0 },
  { 2,1,0,3 },{ 0,0,0,0 },{ 0,0,0,0 },{ 0,0,0,0 },{ 3,1,0,2 },{ 0,0,0,0 },{ 3,2,0,1 },
  { 3,2,1,0 } };


float inline
grad1(const int hash, const float x)
{
  float g = (hash & 7) + 1.0f;
  if (hash & 8)
    g = -1;
  return (g * x);
}

float inline
grad2(const int hash, const float x, const float y)
{
  const int h = hash & 15;
  return x * GRAD3[h][0] + y * GRAD3[h][1];
}

float inline
grad3(const int hash, const float x, const float y, const float z)
{
  const int h = hash & 15;
  return x * GRAD3[h][0] + y * GRAD3[h][1] + z * GRAD3[h][2];
}

float
noise1(float x, const int repeat, const int base)
{
  float fx;
  int i = (int)floorf(x) % repeat;
  int ii = (i + 1) % repeat;
  i = (i & 255) + base;
  ii = (ii & 255) + base;

  x -= floorf(x);
  fx = x*x*x * (x * (x * 6 - 15) + 10);

  return lerp(fx, grad1(PERM[i], x), grad1(PERM[ii], x - 1)) * 0.4f;
}


float
noise2(float x, float y, const float repeatx, const float repeaty, const int base)
{
  float fx, fy;
  int A, AA, AB, B, BA, BB;
  int i = (int)floorf(fmodf(x, repeatx));
  int j = (int)floorf(fmodf(y, repeaty));
  int ii = (int)fmodf(i + 1, repeatx);
  int jj = (int)fmodf(j + 1, repeaty);
  i = (i & 255) + base;
  j = (j & 255) + base;
  ii = (ii & 255) + base;
  jj = (jj & 255) + base;

  x -= floorf(x); y -= floorf(y);
  fx = x*x*x * (x * (x * 6 - 15) + 10);
  fy = y*y*y * (y * (y * 6 - 15) + 10);

  A = PERM[i];
  AA = PERM[A + j];
  AB = PERM[A + jj];
  B = PERM[ii];
  BA = PERM[B + j];
  BB = PERM[B + jj];

  return lerp(fy, lerp(fx, grad2(PERM[AA], x, y),
    grad2(PERM[BA], x - 1, y)),
    lerp(fx, grad2(PERM[AB], x, y - 1),
      grad2(PERM[BB], x - 1, y - 1)));
}

float
noise3(float x, float y, float z, const int repeatx, const int repeaty, const int repeatz,
  const int base)
{
  float fx, fy, fz;
  int A, AA, AB, B, BA, BB;
  int i = (int)floorf(fmodf(x, repeatx));
  int j = (int)floorf(fmodf(y, repeaty));
  int k = (int)floorf(fmodf(z, repeatz));
  int ii = (int)fmodf(i + 1, repeatx);
  int jj = (int)fmodf(j + 1, repeaty);
  int kk = (int)fmodf(k + 1, repeatz);
  i = (i & 255) + base;
  j = (j & 255) + base;
  k = (k & 255) + base;
  ii = (ii & 255) + base;
  jj = (jj & 255) + base;
  kk = (kk & 255) + base;

  x -= floorf(x); y -= floorf(y); z -= floorf(z);
  fx = x*x*x * (x * (x * 6 - 15) + 10);
  fy = y*y*y * (y * (y * 6 - 15) + 10);
  fz = z*z*z * (z * (z * 6 - 15) + 10);

  A = PERM[i];
  AA = PERM[A + j];
  AB = PERM[A + jj];
  B = PERM[ii];
  BA = PERM[B + j];
  BB = PERM[B + jj];

  return lerp(fz, lerp(fy, lerp(fx, grad3(PERM[AA + k], x, y, z),
    grad3(PERM[BA + k], x - 1, y, z)),
    lerp(fx, grad3(PERM[AB + k], x, y - 1, z),
      grad3(PERM[BB + k], x - 1, y - 1, z))),
    lerp(fy, lerp(fx, grad3(PERM[AA + kk], x, y, z - 1),
      grad3(PERM[BA + kk], x - 1, y, z - 1)),
      lerp(fx, grad3(PERM[AB + kk], x, y - 1, z - 1),
        grad3(PERM[BB + kk], x - 1, y - 1, z - 1))));
}

}

float PerlinNoise1D(float X, int Octaves, float Persistence, float Lacunarity, int Repeat, int Base)
{
  if (Octaves == 1) {
    return Perlin::noise1(X, Repeat, Base);
  }
  else if (Octaves > 1) {
    int i;
    float freq = 1.0f;
    float amp = 1.0f;
    float max = 0.0f;
    float total = 0.0f;

    for (i = 0; i < Octaves; i++) {
      total += Perlin::noise1(X * freq, (const int)(Repeat * freq), Base) * amp;
      max += amp;
      freq *= Lacunarity;
      amp *= Persistence;
    }
    return total / max;
  }
  else
  {
    return -1.f;
  }
}


float PerlinNoise2D(k3d::math::TVector<float, 2> const& Vector, int Octaves, float Persistence, float Lacunarity, k3d::math::TVector<float, 2> const& Repeat, int Base)
{
  if (Octaves == 1) {
    return Perlin::noise2(Vector.X(), Vector.Y(), Repeat.X(), Repeat.Y(), Base);
  }
  else if (Octaves > 1) {
    int i;
    float freq = 1.0f;
    float amp = 1.0f;
    float max = 0.0f;
    float total = 0.0f;

    for (i = 0; i < Octaves; i++) {
      total += Perlin::noise2(Vector.X() * freq, Vector.Y() * freq, (const int)(Repeat.X() * freq), (const int)(Repeat.Y() * freq), Base) * amp;
      max += amp;
      freq *= Lacunarity;
      amp *= Persistence;
    }
    return total / max;
  }
  else
  {
    return -1.f;
  }
}

float PerlinNoise3D(k3d::math::TVector<float, 3> const& V, int Octaves, float Persistence, float Lacunarity, k3d::math::TVector<float, 3> const& Repeat, int Base)
{
  if (Octaves == 1) {
    return Perlin::noise3(V[0], V[1], V[2], Repeat.x, Repeat.y, Repeat.z, Base);
  }
  else if (Octaves > 1) {
    int i;
    float freq = 1.0f;
    float amp = 1.0f;
    float max = 0.0f;
    float total = 0.0f;

    for (i = 0; i < Octaves; i++) {
      total += Perlin::noise3(V[0] * freq, V[1] * freq, V[2] * freq, (const int)(Repeat.x * freq), (const int)(Repeat.y * freq), (const int)(Repeat.z * freq), Base) * amp;
      max += amp;
      freq *= Lacunarity;
      amp *= Persistence;
    }
    return total / max;
  }
  else
  {
    return -1.f;
  }
}
