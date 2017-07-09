#ifndef __Reflector_h__
#define __Reflector_h__
#pragma once

namespace k3d
{

class Cursor
{
public:
  Cursor(const CXCursor& c);

  String GetName() const;

  using List = DynArray<Cursor>;

  List GetChildren();

private:
  CXCursor m_Handle;
};


}
#endif