#pragma once

#include "kMath.hpp"

#if K3DPLATFORM_OS_WIN || K3DPLATFORM_OS_LINUX || K3DPLATFORM_OS_MAC
#define __ENABLE_SSE__ 1
#elif K3DPLATFORM_OS_ANDROID
#define __ENABLE_NEON__ 1
#endif

#if __ENABLE_SSE__
#include "kMath_SSE.hpp"
#elif __ENABLE_NEON__
#include "kMath_NEON.hpp"
#endif

NS_MATHLIB_BEGIN
  template <>
  class tVectorN<float, 4>
  {
  public:
    enum { Len = 4 };
    typedef float value_type;

	KFORCE_INLINE tVectorN() { data = simd_set(0.f); }
	KFORCE_INLINE tVectorN(vec4x32 _simd) { data = _simd; }
	KFORCE_INLINE tVectorN(float x, float y, float z, float w) { data = simd_set(x, y, z, w); }

	KFORCE_INLINE explicit tVectorN(const float *ptr)	{ this->init(ptr); }
	KFORCE_INLINE explicit tVectorN(float ones){ data = simd_set(ones); }
	KFORCE_INLINE tVectorN(const tVectorN<float, 3>& vec3, float w)
    {
		data = simd_set(vec3[0],vec3[2],vec3[2],w);
    }

	KFORCE_INLINE void init(const float *ptr) { data = simd_set(ptr[0], ptr[1], ptr[2], ptr[3]); }

    KFORCE_INLINE float& operator [] ( int index )				{ assert( index < 4 && "tVector4 : index < 4 -- Failed !" ); return d[ index ]; }
    KFORCE_INLINE const float operator [] ( int index )const	{ assert( index < 4 && "tVector4 : index < 4 -- Failed !" ); return d[ index ]; }

    operator const float* () const
    {
      return &d[ 0 ];
    }

    KFORCE_INLINE operator tVectorN<float, 3>() const
    {
      return tVectorN<float, 3>( d[0], d[1], d[2] );
    }

    KFORCE_INLINE tVectorN<float, 3> ToVec3()
    {
        return tVectorN<float, 3>( d[0], d[1], d[2] );
    }
	
	KFORCE_INLINE operator vec4x32 () const
    {
      return data;
    }

  private:
    union{
      float d[ 4 ];
      vec4x32 data;
    };
  };

  KFORCE_INLINE float DotProduct( const tVectorN<float, 4>& a, const tVectorN<float, 4>& b )
  {
	  vec4x32 result = simd_dot((vec4x32)a, (vec4x32)b);
#if __ENABLE_SSE__
	  return result.m128_f32[0];
#endif
  }

  KFORCE_INLINE tVectorN<float, 4> operator / (const tVectorN<float, 4> &a, const float factor)
  {
    float f = 1.0f / factor;// little optimize
    vec4x32 res = simd_mul((vec4x32)a, simd_set(f));
    return tVectorN<float, 4>( res );
  }

  template <>
  class tMatrixNxN<float, 4>
  {
  public:
    typedef float value_type;
    enum { N = 4 };
    typedef tVectorN<float, 4> RowType;

    tMatrixNxN() {}

	KFORCE_INLINE tMatrixNxN(RowType &row0, RowType &row1, RowType &row2, RowType &row3)
    {
      data[ 0 ] = row0; data[ 1 ] = row1; data[ 2 ] = row2; data[ 3 ] = row3;
    }

	KFORCE_INLINE tMatrixNxN(std::initializer_list<float>& list)
    {
      int count = 0;
      for(auto i = list.begin(); i!=list.end(); i++)
      {
          data[count/4][count%4] = *i;
          count ++;
          if(count == 16)
              break;
      }
    }

	KFORCE_INLINE explicit tMatrixNxN(value_type raw_data[16])
    {
      data[ 0 ].init( raw_data );
      data[ 1 ].init( raw_data + 4 );
      data[ 2 ].init( raw_data + 8 );
      data[ 3 ].init( raw_data + 12 );
    }

    KFORCE_INLINE RowType& operator [] ( int index ) { assert( index < 4 && "tMatrixNxN : index < _N -- Failed !" );    return data[ index ]; }
    KFORCE_INLINE const RowType operator [] ( int index ) const { assert( index < 4 && "tMatrixNxN : index < _N -- Failed !" ); return data[ index ]; }

	KFORCE_INLINE operator const float* () const
    {
      return data[ 0 ];
    }

  private:
    tVectorN<float, 4> data[ 4 ];
  };
  
  KFORCE_INLINE tMatrixNxN<float, 4> operator * (const tMatrixNxN<float, 4> &a, const tMatrixNxN<float, 4> &b)
  {
    tMatrixNxN<float, 4> res;
	simd_matrix4_mul(&res, (void*)&a, (void*)&b);
    return res;
  }

NS_MATHLIB_END

