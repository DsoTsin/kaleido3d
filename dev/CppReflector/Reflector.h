#ifndef __Reflector_h__
#define __Reflector_h__
#pragma once

#include <Core/Os.h>
namespace k3d
{

    enum class CursorType
    {
        Root,
        Function,
        MacroDefinition,
        Class,
        ClassFunction,
    };

    class Cursor
    {
    public:
      Cursor(const CXCursor& c, CursorType Type = CursorType::Root);

      String GetName() const;
      String GetFullTypeName();

      CursorType GetType() const { return Type; }

      using List = DynArray<Cursor>;

      K3D_DEPRECATED("dddd")
      List GetChildren() KFUNCTION(Y);

      Cursor& operator = (Cursor const& Rhs)
      {
          this->m_Handle = Rhs.m_Handle;
          this->Type = Rhs.Type;
          return *this;
      }

    private:
      CXCursor m_Handle;
      CursorType Type;
    } KCLASS(X);

    class Parser
    {
    public:
        Parser(const char* HeaderFile, const char* _argv[], int argc);
        ~Parser();

        void DoParse();

    private:
        CXIndex             ParIndex;
        CXTranslationUnit   ParUnit;

        DynArray<String>    FunctionNameList;
        DynArray<String>    ClassNameList;
        Os::File            DumpFile;

    };

}
#endif