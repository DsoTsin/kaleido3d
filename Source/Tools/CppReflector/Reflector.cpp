#include <clang-c/Index.h>
#include "Kaleido3D.h"
#include <KTL/String.hpp>
#include <KTL/DynArray.hpp>
#include <Core/LogUtil.h>
#include "Reflector.h"

namespace k3d
{

String cxStr(CXString str)
{
  String _Str(clang_getCString(str));
  clang_disposeString(str);
  return _Str;
}


String GetFullName(CXCursor cursor)
{
  String name;
  while (clang_isDeclaration(clang_getCursorKind(cursor)) != 0)
  {
    String cur = cxStr(clang_getCursorSpelling(cursor));
    if (name.Length()==0)
    {
      name = cur;
    }
    else
    {
      name = cur + String("::") + name;
    }
    cursor = clang_getCursorSemanticParent(cursor);
  }

  return name;
}

Cursor::Cursor(const CXCursor & c)
  : m_Handle(c)
{
}

CXChildVisitResult VisitAnnotation(CXCursor cursor, CXCursor parent, CXClientData data)
{
  switch (clang_getCursorKind(cursor))
  {
  case CXCursor_AnnotateAttr:
    KLOG(Info, annotate, "%s.", cxStr(clang_getCursorSpelling(cursor)).CStr());
    return CXChildVisit_Break;
  }
  return CXChildVisit_Recurse;
}

String Cursor::GetName() const
{
  return cxStr(clang_getCursorDisplayName(m_Handle));
}

Cursor::List Cursor::GetChildren()
{
  Cursor::List list;

  auto visitor = [](CXCursor cursor, CXCursor parent, CXClientData data)
  {
    auto name = GetFullName(cursor);
    
    switch (clang_getCursorKind(cursor))
    {
    case CXCursor_EnumDecl:
      KLOG(Info, Enum, "%s.", name.CStr());
      break;
    case CXCursor_ClassDecl:
    case CXCursor_StructDecl:
    {
      String annotate = cxStr(clang_getCursorUSR(cursor));
      if (annotate.Length() != 0)
      {
        KLOG(Info, Class, "%s. (%s)", name.CStr(), annotate.CStr());
      }
      else
      KLOG(Info, Class, "%s.", name.CStr());
      clang_visitChildren(cursor, VisitAnnotation, nullptr);
      break;
    }
    case CXCursor_FunctionDecl:
      KLOG(Info, Function, "%s.", name.CStr());
      break;
    default:
      break;
    }
    return CXChildVisit_Recurse;
  };

  clang_visitChildren(m_Handle, visitor, &list);
  return list;
}


}