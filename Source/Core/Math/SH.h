#ifndef __SH_h__
#define __SH_h__
#include <Math/kMath.hpp>

namespace SH
{
  using Vec3f = kMath::tVectorN<float, 3>;
  using Mat4f = kMath::tMatrixNxN<float, 4>;

  extern K3D_API float* EvalDir(uint32 Order, Vec3f const& Dir);
  extern K3D_API float* Rotate(uint32 Order, Mat4f const& Mat, const float* In);
}

#endif