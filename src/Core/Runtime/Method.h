#pragma once
#include "Runtime.h"
namespace k3d
{
class K3D_CORE_API Method
{
public:
    Method(String Name, int InternalOffset);
    ~Method();

    void Invoke(StackData& Data);

private:
    int         m_InternalOffset;
    String      m_Name;
    NativeFunc  m_NativeFunc;
    int         m_NumParams;
};
}
