#pragma once
#include "Runtime.h"
namespace k3d
{
class K3D_CORE_API Field
{
public:
    Field(String Name, int InternalOffset);
    ~Field();

    template <typename T>
    T* GetValuePtr(void* Obj) const
    {
        return reinterpret_cast<T*>((char*)Obj + m_InternalOffset);
    }

private:
    int     m_InternalOffset;
    String  m_Name;
};
}