#include "kDebugDrawUtils.h"

kDebugDrawPlane::kDebugDrawPlane()
{
}

kDebugDrawPlane::kDebugDrawPlane(int width, int depth)
{
  SetGridSize(width, depth);
}

void kDebugDrawPlane::SetGridSize(int width, int depth)
{
  m_PlaneVertex[0] = 1.0f*width; m_PlaneVertex[1] = 0.f; m_PlaneVertex[2] = 1.0f*depth;
  m_PlaneVertex[3] = 1.0f*width; m_PlaneVertex[4] = 0.f; m_PlaneVertex[5] = -1.0f*depth;
  m_PlaneVertex[6] = -1.0f*width; m_PlaneVertex[7] = 0.f; m_PlaneVertex[8] = -1.0f*depth;
  m_PlaneVertex[9] = -1.0f*width; m_PlaneVertex[10] = 0.f; m_PlaneVertex[11] = 1.0f*depth;

  m_PlaneIndices = {0, 1, 2, 2, 3, 0};
}

int kDebugDrawPlane::GetIndexNum() const
{
  return 6;
}

int kDebugDrawPlane::GetVertexNum() const
{
  return 4;
}

float *kDebugDrawPlane::GetVertexBuffer()
{
  return m_PlaneVertex;
}

int *kDebugDrawPlane::GetIndexBuffer()
{
  return &m_PlaneIndices[0];
}
