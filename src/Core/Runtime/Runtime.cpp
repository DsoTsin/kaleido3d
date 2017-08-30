#include "CoreMinimal.h"
#include "Runtime.h"
#include "Class.h"
#include "Enum.h"
#include "Field.h"
#include "Method.h"

namespace k3d
{
StackData::StackData() : m_CurOffset(0)
{
}
StackData::~StackData()
{
}
Field::Field(String Name, int InternalOffset)
: m_InternalOffset(InternalOffset)
, m_Name(Name)
{}
Field::~Field()
{}
Method::Method(String Name, int InternalOffset)
: m_InternalOffset(InternalOffset)
, m_Name(Name)
{}
Method::~Method()
{
}
Class::Class(String Name) : m_Name(Name)
{}
Class::~Class()
{}
Field*
Class::FindField(const char* Name, const char* Signature)
{
    return nullptr;
}
Method*
Class::FindMethod(const char* Name, const char* Signature)
{
    return nullptr;
}
U32 Class::GetNumFields() const
{
    return (U32)m_Fields.Count();
}
U32 Class::GetNumMethods()const
{
    return (U32)m_Methods.Count();
}
bool Class::RegisterMethod(const char *MethodName, const char *Signature)
{
    return true;
}
bool Class::RegisterField(const char *MethodName, const char *Signature, int InternalOffset)
{
    auto Fld = new Field(MethodName, InternalOffset);
    m_Fields.Append(Fld);
    return true;
}
Runtime::Runtime()
{}
Runtime::~Runtime()
{}
Class*
Runtime::FindClass(const char* Name)
{
    return nullptr;
}
Method*
Runtime::FindMethod(const char* Name, const char* Signature)
{
    return nullptr;
}
bool
Runtime::Register()
{
    return false;
}

}
