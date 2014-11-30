#pragma once

#include "kMath.hpp"
#include <array>

NS_MATHLIB_BEGIN

typedef tVectorN<float, 3> Vec3f;
typedef tVectorN<float, 4> Vec4f;
typedef tMatrixNxN<float, 4> Mat4f;

const float EPSILON= 0.0001f;

class Plane
{
public:

  enum Face {COPLANAR, FRONT, BACK};

  Plane()
  {
    N = Vec3f(0.f,1.f,0.f);
    d = 0;
  }

  Plane(const Vec3f& normal, const Vec3f& position)
  {
    N = normal;
    d = -DotProduct(N,position);
  }

  ~Plane() {}

  static Plane ConstructFromPoints(const Vec3f& v1, const Vec3f& v2, const Vec3f& v3)
  {
    Plane temp;
    Vec3f e1 = v2-v1;
    Vec3f e2 = v3-v1;
    temp.N = Normalize(CrossProduct(e1,e2));
    temp.d = -DotProduct(temp.N, v1);
    return temp;
  }

  float GetDistance(const Vec3f& p)
  {
    return DotProduct(N,p)+d;
  }

  Face Classify(const Vec3f& p)
  {
    float res = GetDistance(p);
    if( res > EPSILON)
        return FRONT;
    else if(res < EPSILON)
        return BACK;
    else
        return COPLANAR;
  }

  inline Vec4f ToVec4()
  {
    return Vec4f(N,d);
  }

  Vec3f N;
  float d;

};

//! \class  BoundingSphere
//! \brief  A bounding sphere.
class BoundingSphere
{
public:
  BoundingSphere(){}

  BoundingSphere( const Vec3f & position, float radius )
  {
    SetPosition( position );
    SetRadius( radius );
  }

  void SetPosition( const Vec3f & position )
  {
    m_Center[ 0 ] = position[ 0 ];
    m_Center[ 1 ] = position[ 1 ];
    m_Center[ 2 ] = position[ 2 ];
  }

  void SetRadius( float radius )
  {
    m_Center[ 3 ] = radius;
  }

  const Vec3f GetPosition() const
  {
    return Vec3f( m_Center[ 0 ], m_Center[ 1 ], m_Center[ 2 ] );
  }

  const float GetRadius() const
  {
    return m_Center[ 3 ];
  }


  inline BoundingSphere Transform( Mat4f & trans ) {
    Vec4f pos(m_Center.ToVec3(), 1.0f);
    const Vec4f & newCenter = trans*pos;
    return BoundingSphere( newCenter.ToVec3(), m_Center[3]);
  }

private:

  Vec4f m_Center;
};

//! \class  AABB
//! \brief  Axia-Aligned BoundingBox
class AABB
{
public:
  AABB() {}

  AABB( const Vec3f & max_corner, const Vec3f & min_corner )
  {
    SetMinCorner( min_corner ); SetMaxCorner( max_corner );
  }

  void SetMaxCorner( const Vec4f & max_corner )
  {
    m_MaxCorner = max_corner;
    m_Center = (m_MaxCorner + m_MinCorner) / 2.0f;
  }

  void SetMaxCorner( const Vec3f & max_corner )
  {
    SetMaxCorner( Vec4f( max_corner, 0.0f ) );
  }

  void SetMinCorner( const Vec4f & min_corner )
  {
    m_MinCorner = min_corner;
    m_Center = (m_MaxCorner + m_MinCorner) / 2.0f;
  }

  void SetMinCorner( const Vec3f & min_corner )
  {
    SetMinCorner( Vec4f( min_corner, 0.0f ) );
  }

  const Vec3f GetMaxCorner() const
  {
    return (Vec3f)m_MaxCorner;
  }

  const Vec3f GetMinCorner() const
  {
    return (Vec3f)m_MinCorner;
  }

  const Vec3f GetHalf() const
  {
    return (m_MaxCorner - m_MinCorner)/2.0;
  }

  const Vec3f GetCenter() const
  {
    return m_Center;
  }

  AABB& operator |= (AABB const & rhs)
  {
    m_MinCorner = Minimize<float, 3>(rhs.GetMinCorner(), m_MinCorner);
    m_MaxCorner = Maximize<float, 3>(rhs.GetMaxCorner(), m_MaxCorner);
    return *this;
  }

  inline bool Inside( const Vec3f & point )
  {
    return (point[0] < m_MaxCorner[0]) && (point[0] > m_MinCorner[0]) &&
        ( point[1] > m_MinCorner[1] ) && ( point[1] < m_MaxCorner[1] ) &&
        ( point[2] > m_MinCorner[2] ) && ( point[2] < m_MaxCorner[2] );
  }
  
  inline bool Inside( const AABB &bbox )
  {
    if ( !Inside( bbox.m_Center ) ) return false;

    for ( int i = 0; i < 3; i++ )
    {
      if ( m_MaxCorner[ i ] < bbox.m_MaxCorner[ i ] )
        return false;
      if ( bbox.m_MinCorner[ i ] > m_MinCorner[ i ] )
        return false;
    }
    return true;
  }

  inline AABB Transform( Mat4f & trans ) {
    Vec4f pos(m_Center, 1.0f);
    const Vec4f & newCenter = trans*pos;
    return AABB( newCenter.ToVec3(), m_Center[3]);
  }

private:
  Vec3f m_MaxCorner;
  Vec3f m_MinCorner;
  Vec3f m_Center;
};

NS_MATHLIB_END
