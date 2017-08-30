#ifndef __Noise_h__
#define __Noise_h__

extern K3D_CORE_API float PerlinNoise1D(float X, int Octave, float Persistance, float Lacunarity, int Repeat, int Base);
extern K3D_CORE_API float PerlinNoise2D(k3d::math::TVector<float,2> const& Vector, int Octaves, float Persistence, float Lacunarity, k3d::math::TVector<float, 2> const& Repeat, int Base);
extern K3D_CORE_API float PerlinNoise3D(k3d::math::TVector<float, 2> const& Vector, int Octaves, float Persistence, float Lacunarity, k3d::math::TVector<float, 2> const& Repeat, int Base);

#endif