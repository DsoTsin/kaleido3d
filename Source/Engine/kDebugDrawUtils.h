#pragma once

#include "k3dSceneObject.h"

//! \class	kDebugDrawPlane
//! \brief	A plane for debug draw purpose.
class kDebugDrawPlane : public kSObject
{
public:
  kDebugDrawPlane();

  //! \fn	kDebugDrawPlane::kDebugDrawPlane(int width, int depth);
  //! \brief	Init Plane With Size
  //! \param	width	The width.
  //! \param	depth	The depth.
  kDebugDrawPlane(int width, int depth);

  void SetGridSize(int width, int depth);

  int GetIndexNum() const;
  int GetVertexNum() const;

  float * GetVertexBuffer();
  int *   GetIndexBuffer();

private:
  float                 m_PlaneVertex[4*3];
  std::array<int, 6>    m_PlaneIndices;
};

class kDebugDrawBox : public kSObject
{
public:
  kDebugDrawBox(int width, int height, int depth);
};

class kDebugDrawUtils
{

};
