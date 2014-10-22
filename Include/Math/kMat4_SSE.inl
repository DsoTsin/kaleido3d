#pragma once
#ifndef __kSSEMat4_hpp__
#define __kSSEMat4_hpp__

#include "kMath.hpp"
#include <xmmintrin.h>

#define _MM_PERM2_X		0
#define _MM_PERM2_Y		1
#define _MM_PERM2_Z		2
#define _MM_PERM2_W		3
#define _MM_PERM2(X,Y,Z,W) _MM_SHUFFLE(_MM_PERM2_ ## W,_MM_PERM2_ ## Z,_MM_PERM2_ ## Y,_MM_PERM2_ ## X)
#define _MM_SWIZZLE(V,X,Y,Z,W) _mm_shuffle_ps(V,V,_MM_PERM2(X,Y,Z,W))

  inline __m128 _mm_rcp_ss_nr( __m128 v ) {
    __m128 iv = _mm_rcp_ss( v );
    return _mm_sub_ss( _mm_add_ss( iv, iv ), _mm_mul_ss( v, _mm_mul_ss( iv, iv ) ) );
  }

//NS_MATHLIB_BEGIN
  template <>
  class tVectorN<float, 4>
  {
  public:
    enum { Len = 4 };
    typedef float value_type;

    tVectorN() { data = _mm_setzero_ps(); }
    tVectorN( __m128 _simd ) { data = _simd; }
    tVectorN( float x, float y, float z, float w ) { data = _mm_set_ps( w, z, y, x ); }
    explicit tVectorN( const float *ptr )	{ this->init( ptr ); }
    explicit tVectorN( float ones ){ data = _mm_set1_ps( ones ); }
    tVectorN( const tVectorN<float, 3>& vec3, float w ) 
    {
      d[ 0 ] = vec3[ 0 ]; d[ 1 ] = vec3[ 2 ]; d[ 2 ] = vec3[ 2 ]; d[3] = w;
    }

    void init( const float *ptr ) { data = _mm_set_ps( ptr[ 3 ], ptr[ 2 ], ptr[ 1 ], ptr[ 0 ] ); }

    inline float& operator [] ( int index )				{ assert( index < 4 && "tVector4 : index < 4 -- Failed !" ); return d[ index ]; }
    inline const float operator [] ( int index )const	{ assert( index < 4 && "tVector4 : index < 4 -- Failed !" ); return d[ index ]; }

    operator const float* () const
    {
      return &d[ 0 ];
    }

    operator tVectorN<float, 3>() const
    {
      return tVectorN<float, 3>( d[0], d[1], d[2] );
    }

    inline tVectorN<float, 3> ToVec3()
    {
        return tVectorN<float, 3>( d[0], d[1], d[2] );
    }

    friend float dotProduct( const tVectorN<float, 4>&, const tVectorN<float, 4>& );
    friend tVectorN<float, 4> operator / (const tVectorN<float, 4> &, const float factor);

    operator __m128 () const
    {
      return data;
    }

  private:
    union{
      float d[ 4 ];
      __m128 data;
    };
  };

  inline float DotProduct( const tVectorN<float, 4>& a, const tVectorN<float, 4>& b )
  {
    return a[ 0 ] * b[ 0 ] + a[ 1 ] * b[ 1 ] + a[ 2 ] * b[ 2 ] + a[ 3 ] * b[ 3 ];
  }

  inline tVectorN<float, 4> operator / (const tVectorN<float, 4> &a, const float factor)
  {
    float f = 1.0f / factor;// little optimize
    __m128 res = _mm_mul_ps(a.data, _mm_set1_ps(f));
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

    tMatrixNxN( RowType &row0, RowType &row1, RowType &row2, RowType &row3 )
    {
      data[ 0 ] = row0; data[ 1 ] = row1; data[ 2 ] = row2; data[ 3 ] = row3;
    }

    tMatrixNxN( std::initializer_list<float>& list)
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
//    explicit tMatrixNxN( value_type row[][ 4 ] )
//    {
//      data[ 0 ].init( row[ 0 ] );
//      data[ 1 ].init( row[ 1 ] );
//      data[ 2 ].init( row[ 2 ] );
//      data[ 3 ].init( row[ 3 ] );
//    }

    explicit tMatrixNxN( value_type raw_data[ 16 ] )
    {
      data[ 0 ].init( raw_data );
      data[ 1 ].init( raw_data + 4 );
      data[ 2 ].init( raw_data + 8 );
      data[ 3 ].init( raw_data + 12 );
    }

    inline RowType& operator [] ( int index ) { assert( index < 4 && "tMatrixNxN : index < _N -- Failed !" );    return data[ index ]; }
    inline const RowType operator [] ( int index ) const { assert( index < 4 && "tMatrixNxN : index < _N -- Failed !" ); return data[ index ]; }

    operator const float* () const
    {
      return data[ 0 ];
    }

    friend inline tMatrixNxN<float, 4> Rotate( );
    friend inline tMatrixNxN<float, 4> Transpose( const tMatrixNxN<float, 4> & a );
    friend inline tMatrixNxN<float, 4> operator * (const tMatrixNxN<float, 4> &a, const tMatrixNxN<float, 4> &b);
//    friend inline tMatrixNxN<float, 4> operator * (const tMatrixNxN<float, 4> &a, const tVectorN<float, 4> &b);
    tMatrixNxN<float, 4> Inverse()
    {
      tMatrixNxN<float, 4> result; 
      __m128 res_0 = _mm_shuffle_ps( data[0], data[1], _MM_PERM2( X, Y, X, Y ) );
      __m128 res_1 = _mm_shuffle_ps( data[0], data[1], _MM_PERM2( Z, W, Z, W ) );
      __m128 res_2 = _mm_shuffle_ps( data[2], data[3], _MM_PERM2( X, Y, X, Y ) );
      __m128 res_3 = _mm_shuffle_ps( data[2], data[3], _MM_PERM2( Z, W, Z, W ) );
      __m128 row_0 = _mm_shuffle_ps( res_0, res_2, _MM_PERM2( X, Z, X, Z ) );
      __m128 row_1 = _mm_shuffle_ps( res_2, res_0, _MM_PERM2( Y, W, Y, W ) );
      __m128 row_2 = _mm_shuffle_ps( res_1, res_3, _MM_PERM2( X, Z, X, Z ) );
      __m128 row_3 = _mm_shuffle_ps( res_3, res_1, _MM_PERM2( Y, W, Y, W ) );
      __m128 temp = _mm_mul_ps( row_2, row_3 );
      temp = _MM_SWIZZLE( temp, Y, X, W, Z );
      res_0 = _mm_mul_ps( row_1, temp );
      res_1 = _mm_mul_ps( row_0, temp );
      temp = _MM_SWIZZLE( temp, Z, W, X, Y );
      res_0 = _mm_sub_ps( _mm_mul_ps( row_1, temp ), res_0 );
      res_1 = _mm_sub_ps( _mm_mul_ps( row_0, temp ), res_1 );
      res_1 = _MM_SWIZZLE( res_1, Z, W, X, Y );
      temp = _mm_mul_ps( row_1, row_2 );
      temp = _MM_SWIZZLE( temp, Y, X, W, Z );
      res_0 = _mm_add_ps( _mm_mul_ps( row_3, temp ), res_0 );
      res_3 = _mm_mul_ps( row_0, temp );
      temp = _MM_SWIZZLE( temp, Z, W, X, Y );
      res_0 = _mm_sub_ps( res_0, _mm_mul_ps( row_3, temp ) );
      res_3 = _mm_sub_ps( _mm_mul_ps( row_0, temp ), res_3 );
      res_3 = _MM_SWIZZLE( res_3, Z, W, X, Y );
      temp = _mm_mul_ps( _MM_SWIZZLE( row_1, Z, W, X, Y ), row_3 );
      temp = _MM_SWIZZLE( temp, Y, X, W, Z );
      row_2 = _MM_SWIZZLE( row_2, Z, W, X, Y );
      res_0 = _mm_add_ps( _mm_mul_ps( row_2, temp ), res_0 );
      res_2 = _mm_mul_ps( row_0, temp );
      temp = _MM_SWIZZLE( temp, Z, W, X, Y );
      res_0 = _mm_sub_ps( res_0, _mm_mul_ps( row_2, temp ) );
      res_2 = _mm_sub_ps( _mm_mul_ps( row_0, temp ), res_2 );
      res_2 = _MM_SWIZZLE( res_2, Z, W, X, Y );
      temp = _mm_mul_ps( row_0, row_1 );
      temp = _MM_SWIZZLE( temp, Y, X, W, Z );
      res_2 = _mm_add_ps( _mm_mul_ps( row_3, temp ), res_2 );
      res_3 = _mm_sub_ps( _mm_mul_ps( row_2, temp ), res_3 );
      temp = _MM_SWIZZLE( temp, Z, W, X, Y );
      res_2 = _mm_sub_ps( _mm_mul_ps( row_3, temp ), res_2 );
      res_3 = _mm_sub_ps( res_3, _mm_mul_ps( row_2, temp ) );
      temp = _mm_mul_ps( row_0, row_3 );
      temp = _MM_SWIZZLE( temp, Y, X, W, Z );
      res_1 = _mm_sub_ps( res_1, _mm_mul_ps( row_2, temp ) );
      res_2 = _mm_add_ps( _mm_mul_ps( row_1, temp ), res_2 );
      temp = _MM_SWIZZLE( temp, Z, W, X, Y );
      res_1 = _mm_add_ps( _mm_mul_ps( row_2, temp ), res_1 );
      res_2 = _mm_sub_ps( res_2, _mm_mul_ps( row_1, temp ) );
      temp = _mm_mul_ps( row_0, row_2 );
      temp = _MM_SWIZZLE( temp, Y, X, W, Z );
      res_1 = _mm_add_ps( _mm_mul_ps( row_3, temp ), res_1 );
      res_3 = _mm_sub_ps( res_3, _mm_mul_ps( row_1, temp ) );
      temp = _MM_SWIZZLE( temp, Z, W, X, Y );
      res_1 = _mm_sub_ps( res_1, _mm_mul_ps( row_3, temp ) );
      res_3 = _mm_add_ps( _mm_mul_ps( row_1, temp ), res_3 );
      __m128 det = _mm_mul_ps( row_0, res_0 );
      det = _mm_add_ps( det, _MM_SWIZZLE( det, Y, X, W, Z ) );
      det = _mm_add_ss( det, _MM_SWIZZLE( det, Z, W, X, Y ) );
      temp = _MM_SWIZZLE( _mm_rcp_ss_nr( det ), X, X, X, X );
      result[0] = _mm_mul_ps( res_0, temp );
      result[1] = _mm_mul_ps( res_1, temp );
      result[2] = _mm_mul_ps( res_2, temp );
      result[3] = _mm_mul_ps( res_3, temp );
      return result;
    }

  private:
    tVectorN<float, 4> data[ 4 ];
  };

  inline tMatrixNxN<float, 4> Transpose( const tMatrixNxN<float, 4> &a ){
    __m128 *in = (__m128 *)&a.data[0];
    __m128 tmp0 = _mm_shuffle_ps( in[ 0 ], in[ 1 ], 0x44 );
    __m128 tmp2 = _mm_shuffle_ps( in[ 0 ], in[ 1 ], 0xEE );
    __m128 tmp1 = _mm_shuffle_ps( in[ 2 ], in[ 3 ], 0x44 );
    __m128 tmp3 = _mm_shuffle_ps( in[ 2 ], in[ 3 ], 0xEE );

    tMatrixNxN<float, 4> result;
    __m128 *out = (__m128 *)&result.data[0];
    out[ 0 ] = _mm_shuffle_ps( tmp0, tmp1, 0x88 );
    out[ 1 ] = _mm_shuffle_ps( tmp0, tmp1, 0xDD );
    out[ 2 ] = _mm_shuffle_ps( tmp2, tmp3, 0x88 );
    out[ 3 ] = _mm_shuffle_ps( tmp2, tmp3, 0xDD );
    return result;
  }

  inline tMatrixNxN<float, 4> operator * (const tMatrixNxN<float, 4> &a, const tMatrixNxN<float, 4> &b)
  {
    tMatrixNxN<float, 4> res;
    __m128 *in1 = (__m128*)&a.data[ 0 ];
    __m128 *in2 = (__m128*)&b.data[ 0 ];
    __m128 *out = (__m128*)&res.data[ 0 ];
    {
      __m128 e0 = _mm_shuffle_ps( in2[ 0 ], in2[ 0 ], _MM_SHUFFLE( 0, 0, 0, 0 ) );
      __m128 e1 = _mm_shuffle_ps( in2[ 0 ], in2[ 0 ], _MM_SHUFFLE( 1, 1, 1, 1 ) );
      __m128 e2 = _mm_shuffle_ps( in2[ 0 ], in2[ 0 ], _MM_SHUFFLE( 2, 2, 2, 2 ) );
      __m128 e3 = _mm_shuffle_ps( in2[ 0 ], in2[ 0 ], _MM_SHUFFLE( 3, 3, 3, 3 ) );

      __m128 m0 = _mm_mul_ps( in1[ 0 ], e0 );
      __m128 m1 = _mm_mul_ps( in1[ 1 ], e1 );
      __m128 m2 = _mm_mul_ps( in1[ 2 ], e2 );
      __m128 m3 = _mm_mul_ps( in1[ 3 ], e3 );

      __m128 a0 = _mm_add_ps( m0, m1 );
      __m128 a1 = _mm_add_ps( m2, m3 );
      __m128 a2 = _mm_add_ps( a0, a1 );

      out[ 0 ] = a2;
    }

    {
      __m128 e0 = _mm_shuffle_ps( in2[ 1 ], in2[ 1 ], _MM_SHUFFLE( 0, 0, 0, 0 ) );
      __m128 e1 = _mm_shuffle_ps( in2[ 1 ], in2[ 1 ], _MM_SHUFFLE( 1, 1, 1, 1 ) );
      __m128 e2 = _mm_shuffle_ps( in2[ 1 ], in2[ 1 ], _MM_SHUFFLE( 2, 2, 2, 2 ) );
      __m128 e3 = _mm_shuffle_ps( in2[ 1 ], in2[ 1 ], _MM_SHUFFLE( 3, 3, 3, 3 ) );

      __m128 m0 = _mm_mul_ps( in1[ 0 ], e0 );
      __m128 m1 = _mm_mul_ps( in1[ 1 ], e1 );
      __m128 m2 = _mm_mul_ps( in1[ 2 ], e2 );
      __m128 m3 = _mm_mul_ps( in1[ 3 ], e3 );

      __m128 a0 = _mm_add_ps( m0, m1 );
      __m128 a1 = _mm_add_ps( m2, m3 );
      __m128 a2 = _mm_add_ps( a0, a1 );

      out[ 1 ] = a2;
    }

    {
      __m128 e0 = _mm_shuffle_ps( in2[ 2 ], in2[ 2 ], _MM_SHUFFLE( 0, 0, 0, 0 ) );
      __m128 e1 = _mm_shuffle_ps( in2[ 2 ], in2[ 2 ], _MM_SHUFFLE( 1, 1, 1, 1 ) );
      __m128 e2 = _mm_shuffle_ps( in2[ 2 ], in2[ 2 ], _MM_SHUFFLE( 2, 2, 2, 2 ) );
      __m128 e3 = _mm_shuffle_ps( in2[ 2 ], in2[ 2 ], _MM_SHUFFLE( 3, 3, 3, 3 ) );

      __m128 m0 = _mm_mul_ps( in1[ 0 ], e0 );
      __m128 m1 = _mm_mul_ps( in1[ 1 ], e1 );
      __m128 m2 = _mm_mul_ps( in1[ 2 ], e2 );
      __m128 m3 = _mm_mul_ps( in1[ 3 ], e3 );

      __m128 a0 = _mm_add_ps( m0, m1 );
      __m128 a1 = _mm_add_ps( m2, m3 );
      __m128 a2 = _mm_add_ps( a0, a1 );

      out[ 2 ] = a2;
    }

    {
      //(__m128&)_mm_shuffle_epi32(__m128i&)in2[0], _MM_SHUFFLE(3, 3, 3, 3))
      __m128 e0 = _mm_shuffle_ps( in2[ 3 ], in2[ 3 ], _MM_SHUFFLE( 0, 0, 0, 0 ) );
      __m128 e1 = _mm_shuffle_ps( in2[ 3 ], in2[ 3 ], _MM_SHUFFLE( 1, 1, 1, 1 ) );
      __m128 e2 = _mm_shuffle_ps( in2[ 3 ], in2[ 3 ], _MM_SHUFFLE( 2, 2, 2, 2 ) );
      __m128 e3 = _mm_shuffle_ps( in2[ 3 ], in2[ 3 ], _MM_SHUFFLE( 3, 3, 3, 3 ) );

      __m128 m0 = _mm_mul_ps( in1[ 0 ], e0 );
      __m128 m1 = _mm_mul_ps( in1[ 1 ], e1 );
      __m128 m2 = _mm_mul_ps( in1[ 2 ], e2 );
      __m128 m3 = _mm_mul_ps( in1[ 3 ], e3 );

      __m128 a0 = _mm_add_ps( m0, m1 );
      __m128 a1 = _mm_add_ps( m2, m3 );
      __m128 a2 = _mm_add_ps( a0, a1 );

      out[ 3 ] = a2;
    }
    return res;
  }

 /* inline tVectorN<float, 4> operator * (const tMatrixNxN<float, 4> &a, const tVectorN<float, 4> &b)
  {
    __m128 *m = (__m128*)&a[ 0 ];
    __m128 v = b.SSEData();
    __m128 v0 = _mm_shuffle_ps( v, v, _MM_SHUFFLE( 0, 0, 0, 0 ) );
    __m128 v1 = _mm_shuffle_ps( v, v, _MM_SHUFFLE( 1, 1, 1, 1 ) );
    __m128 v2 = _mm_shuffle_ps( v, v, _MM_SHUFFLE( 2, 2, 2, 2 ) );
    __m128 v3 = _mm_shuffle_ps( v, v, _MM_SHUFFLE( 3, 3, 3, 3 ) );

    __m128 m0 = _mm_mul_ps( m[ 0 ], v0 );
    __m128 m1 = _mm_mul_ps( m[ 1 ], v1 );
    __m128 m2 = _mm_mul_ps( m[ 2 ], v2 );
    __m128 m3 = _mm_mul_ps( m[ 3 ], v3 );

    __m128 a0 = _mm_add_ps( m0, m1 );
    __m128 a1 = _mm_add_ps( m2, m3 );
    __m128 a2 = _mm_add_ps( a0, a1 );
    return tVectorN<float, 4>( &a2.m128_f32[ 0 ] );
  }*/
//NS_MATHLIB_END
#endif
