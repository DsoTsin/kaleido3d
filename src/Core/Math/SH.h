#ifndef __SH_h__
#define __SH_h__

namespace k3d 
{
    namespace SH
    {
        using Vec3f = k3d::math::TVector<float, 3>;
        using Mat4f = k3d::math::TMatrixN<float, 4>;

        extern K3D_CORE_API float* EvalDir(U32 Order, Vec3f const& Dir);
        extern K3D_CORE_API float* Rotate(U32 Order, Mat4f const& Mat, const float* In);
    }
}
#endif