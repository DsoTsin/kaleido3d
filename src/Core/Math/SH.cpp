#include "CoreMinimal.h"
#include "SH.h"
namespace k3d
{
    namespace SH
    {
        namespace HardCode
        {
            const int MaxOrder = 4;
            const float S_PI = 3.141592654f;
            const float S_INVPI = 0.318309886f;
        } // HardCoded

        float * EvalDir(U32 Order, Vec3f const & Dir)
        {
            return nullptr;
        }

        float* Rotate(U32 Order, Mat4f const& Mat, const float* In)
        {
            return nullptr;
        }
    }
}