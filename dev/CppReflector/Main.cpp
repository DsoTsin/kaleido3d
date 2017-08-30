#if WIN32
#pragma comment(linker, "/SUBSYSTEM:CONSOLE")
#endif
#include <iostream>
#include <clang-c/Index.h>
#include <Core/Kaleido3D.h>
#include <Core/KTL/String.hpp>
#include <Core/KTL/DynArray.hpp>
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
  k3d::Parser P(file, arv_, argc);
  P.DoParse();
  return 0;
}