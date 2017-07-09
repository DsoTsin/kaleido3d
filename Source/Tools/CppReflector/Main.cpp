#if WIN32
#pragma comment(linker, "/SUBSYSTEM:CONSOLE")
#endif
#include <iostream>
#include <clang-c/Index.h>
#include "Kaleido3D.h"
#include <KTL/String.hpp>
#include <KTL/DynArray.hpp>
#include <Core/LogUtil.h>
#include "Reflector.h"
using namespace std;

ostream& operator<<(ostream& stream, const CXString& str)
{
  stream << clang_getCString(str);
  clang_disposeString(str);
  return stream;
}

int main(int argc, const char* argv[])
{
  // example command: D:\project\kaleido3d\Include\Interface\IRHI.h -- -x c++ -std=c++14 -g -DK3D_CPP_REFLECTOR
  const char * file = argv[1];
  const char ** arv_ = argv + 3;
  CXIndex index = clang_createIndex(0, 0);
  CXTranslationUnit unit = clang_parseTranslationUnit(
    index,
    file, arv_, argc-3,
    nullptr, 0,
    CXTranslationUnit_None);
  if (unit == nullptr)
  {
    KLOG(Fatal, CppReflector, "Unable to parse translation unit. Quitting.");
    exit(-1);
  }
  k3d::Cursor cur(clang_getTranslationUnitCursor(unit));
  auto childs = cur.GetChildren();
  for (auto c : childs)
  {
    auto string = c.GetName();
    (string);
  }
  clang_disposeTranslationUnit(unit);
  clang_disposeIndex(index);
  return 0;
}