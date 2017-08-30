#pragma once

#include "Object.h"

namespace k3d
{
class Class;
class Method;

class K3D_CORE_API StackData
{
public:
    StackData();
    ~StackData();

    template <typename T>
    T* Read()
    {
        T* CurPtr = reinterpret_cast<T*>(m_Data + m_CurOffset);
        m_CurOffset += sizeof(T);
        return CurPtr;
    }

    IObject* ReadObject()
    {
        IObject* CurPtr = reinterpret_cast<IObject*>(m_Data + m_CurOffset);
        m_CurOffset += sizeof(IObject*);
        return CurPtr;
    }

private:
    U64 m_CurOffset;
    U64 m_StackSize;
    U8* m_Data;
};

typedef void (IObject::*NativeFunc)(StackData& Data);

class K3D_CORE_API Runtime
{
public:
    Runtime();
    ~Runtime();

    Class*  FindClass(const char* Name);
    Method* FindMethod(const char* Name, const char* Signature);

    bool    Register();

private:
};
}