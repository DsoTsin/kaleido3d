#include "k3dCamera.h"
#include <Core/k3dInputEvent.h>
#include <Core/k3dDbg.h>

k3dCamera::k3dCamera()
{}

k3dCamera::~k3dCamera()
{}

void k3dCamera::Update()
{}

void k3dCamera::Rotate(const float yaw, const float pitch, const float roll)
{
  m_Yaw     = kMath::ToRadian(yaw);
  m_Pitch   = kMath::ToRadian(pitch);
  m_Roll    = kMath::ToRadian(roll);
  Update();
}

void k3dCamera::SetProjection(float fovy, float ratio, float near_plane, float far_plane)
{
  m_ProjectionMatrix = kMath::Perspective(fovy, ratio, near_plane, far_plane);
  m_Znear = near_plane;
  m_Zfar = far_plane;
  m_Fov = fovy;
  m_AspectRatio = ratio;
}

const kMath::Mat4f k3dCamera::GetViewMatrix() const
{
  return m_ViewMatrix;
}

const kMath::Mat4f k3dCamera::GetProjectionMatrix() const
{
  return m_ProjectionMatrix;
}

void k3dCamera::SetPosition(const kMath::Vec3f &v)
{
  m_CameraPosition = v;
}

const kMath::Vec3f k3dCamera::GetPosition() const
{
  return m_CameraPosition;
}

void k3dCamera::SetFOV(const float fov)
{
  m_Fov = fov;
}

const float k3dCamera::GetFOV() const
{
  return m_Fov;
}

const float k3dCamera::GetAspectRatio() const
{
  return m_AspectRatio;
}

void k3dCamera::CalcFrustumPlanes()
{
  kMath::Vec3f cN = m_CameraPosition + m_LookVector*m_Znear;
  kMath::Vec3f cF = m_CameraPosition + m_LookVector*m_Zfar;

  float Hnear = 2.0f * ::tan(kMath::ToRadian( m_Fov / 2.0f) ) * m_Znear;
  float Wnear = Hnear * m_AspectRatio;
  float Hfar = 2.0f * ::tan(kMath::ToRadian( m_Fov / 2.0f)) * m_Zfar;
  float Wfar = Hfar * m_AspectRatio;
  float hHnear = Hnear/2.0f;
  float hWnear = Wnear/2.0f;
  float hHfar = Hfar/2.0f;
  float hWfar = Wfar/2.0f;

  m_FarPts[0] = cF + m_UpVector*hHfar - m_RightVector*hWfar;
  m_FarPts[1] = cF - m_UpVector*hHfar - m_RightVector*hWfar;
  m_FarPts[2] = cF - m_UpVector*hHfar + m_RightVector*hWfar;
  m_FarPts[3] = cF + m_UpVector*hHfar + m_RightVector*hWfar;

  m_NearPts[0] = cN + m_UpVector*hHnear - m_RightVector*hWnear;
  m_NearPts[1] = cN - m_UpVector*hHnear - m_RightVector*hWnear;
  m_NearPts[2] = cN - m_UpVector*hHnear + m_RightVector*hWnear;
  m_NearPts[3] = cN + m_UpVector*hHnear + m_RightVector*hWnear;

  m_Planes[0] = kMath::Plane::ConstructFromPoints(m_NearPts[3],m_NearPts[0],m_FarPts[0]);
  m_Planes[1] = kMath::Plane::ConstructFromPoints(m_NearPts[1],m_NearPts[2],m_FarPts[2]);
  m_Planes[2] = kMath::Plane::ConstructFromPoints(m_NearPts[0],m_NearPts[1],m_FarPts[1]);
  m_Planes[3] = kMath::Plane::ConstructFromPoints(m_NearPts[2],m_NearPts[3],m_FarPts[2]);
  m_Planes[4] = kMath::Plane::ConstructFromPoints(m_NearPts[0],m_NearPts[3],m_NearPts[2]);
  m_Planes[5] = kMath::Plane::ConstructFromPoints(m_FarPts[3] ,m_FarPts[0] ,m_FarPts[1]);
}

bool k3dCamera::IsPointInFrustum(const kMath::Vec3f &point)
{
  for(int i=0; i < 6; i++)
  {
    if (m_Planes[i].GetDistance(point) < 0)
      return false;
  }
  return true;
}

bool k3dCamera::IsSphereInFrustum(const kMath::BoundingSphere &sphere)
{
  for(int i=0; i < 6; i++)
  {
    float d = m_Planes[i].GetDistance(sphere.GetPosition());
    if ( d < -sphere.GetRadius())
      return false;
  }
  return true;
}

BoundType k3dCamera::IntersectBox(const kMath::AABB &aabb)
{
  kMath::Vec3f p=aabb.GetMinCorner(), n=aabb.GetMaxCorner();
  bool intersect = false;

  for(int i=0; i < 6; i++)
  {
    kMath::Vec3f N = m_Planes[i].N;

    if(N.x>=0)
    {
      p.x = n.x;
      n.x = p.x;
    }
    if(N.y>=0)
    {
      p.y = n.y;
      n.y = p.y;
    }
    if(N.z>=0)
    {
      p.z = n.z;
      n.z = p.z;
    }

    if ( m_Planes[i].GetDistance(p) < 0 )
    {
      intersect = false;
    }

    if( m_Planes[i].GetDistance(n) < 0 )
    {
      return BO_NO;
    }
  }

  return intersect? BO_PARTIAL : BO_YES;
}

void k3dCamera::GetFrustumPlanes(kMath::Vec4f planes[])
{
    for(int i=0; i<6; i++)
    {
       planes[i] =  m_Planes[i].ToVec4();
    }
}
