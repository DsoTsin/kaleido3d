#ifndef __Noise_h__
#define __Noise_h__
#include <Math/kMath.hpp>

extern K3D_API float PerlinNoise1D(float X, int Octave, float Persistance, float Lacunarity, int Repeat, int Base);
extern K3D_API float PerlinNoise2D(kMath::tVectorN<float,2> const& Vector, int Octaves, float Persistence, float Lacunarity, kMath::tVectorN<int, 2> const& Repeat, int Base);
extern K3D_API float PerlinNoise3D(kMath::tVectorN<float, 3> const& Vector, int Octaves, float Persistence, float Lacunarity, kMath::tVectorN<int, 3> const& Repeat, int Base);

#endif