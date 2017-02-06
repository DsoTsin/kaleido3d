/***********************************************
 *  Kaleido3D Math Library
 *  Author  : Qin Zhou
 *  Date    : 2014/1/31
 *  Email   : dsotsen@gmail.com
 ***********************************************/
#pragma once
#ifndef __KMATH_HPP__
#define __KMATH_HPP__

#define NS_MATHLIB_BEGIN namespace kMath {
#define NS_MATHLIB_END }

#include "kTypeTrait.hpp"
#include "../Kaleido3D.h"
#include "../Config/OSHeaders.h"

#if K3DCOMPILER_CLANG || K3DCOMPILER_GCC
#include <cmath>
#elif K3DCOMPILER_MSVC
#pragma warning(disable:4201) // disable anonymous union warnings
#endif

NS_MATHLIB_BEGIN

// Vector Math ---------------------------------------------------------------------------------
template <class T> KFORCE_INLINE typename T::value_type DotProduct( T a, T b );

template <typename T, int N>
struct tVectorN;

template <typename T, int N>
struct tVectorN
{
  tVectorN() { ::memset( m_data, 0, N*sizeof(T) ); }
  template <class U>
  tVectorN( const U *ptr )	{ this->template init<U>( ptr ); }

  template <class U>
  void init( U *s_offset )
  {
    assert( sizeof(U) <= sizeof(T) && s_offset );
    ::memcpy( m_data, s_offset, N*sizeof(U) );
  }
  template <class U>
  void init( U *s_offset, int n )
  {
    assert( s_offset && n <= N && n*sizeof(U) <= N*sizeof(T) );
    ::memcpy( m_data, s_offset, n*sizeof(U) );
  }

  enum { Len = N };
  typedef T value_type;
  KFORCE_INLINE T& operator [] ( int index )                  { assert( index < N && "tVectorN : index < N -- Failed !" ); return m_data[ index ]; }
  KFORCE_INLINE const T operator [] ( int index )const	{ assert( index < N && "tVectorN : index < N -- Failed !" ); return m_data[ index ]; }

  operator const T* ()
  {
    return &m_data[ 0 ];
  }

protected:
  T m_data[ N ];
};

template <typename T, int N>
KFORCE_INLINE const T Length(const tVectorN<T, N>& rhs) {
  return T(sqrt( 1.0*DotProduct( rhs, rhs ) ));
}

template <typename T, int N>
tVectorN<T, N> Maximize(tVectorN<T, N> const &v1, tVectorN<T, N> const &v2)
{
  tVectorN<T, N> result;
  for(int i=0; i<N; i++)
    result[i] = (v1[i]>v2[i])?v1[i]:v2[i];
  return result;
}

template <typename T, int N>
tVectorN<T, N> Minimize(tVectorN<T, N> const &v1, tVectorN<T, N> const &v2)
{
  tVectorN<T, N> result;
  for(int i=0; i<N; i++)
    result[i] = (v1[i]<v2[i])?v1[i]:v2[i];
  return result;
}

///////////////////////////Operator Implement//////////////////////////////
template <typename T, int N>
KFORCE_INLINE tVectorN<T, N> operator + (tVectorN<T, N> const & a, tVectorN<T, N> const & b)
{
  tVectorN<T, N> result;
  for (int i = 0; i < N; i++) result[i] = a[i] + b[i];
  return result;
}
template <typename T, int N>
KFORCE_INLINE tVectorN<T, N> operator - (tVectorN<T, N> const & a, tVectorN<T, N> const & b)
{
  tVectorN<T, N> result;
  for (int i = 0; i < N; i++) result[i] = a[i] - b[i];
  return result;
}

template <typename T, int N>
KFORCE_INLINE tVectorN<T, N> operator + (tVectorN<T, N> const & a, T const & b)
{
  tVectorN<T, N> result;
  for (int i = 0; i < N; i++) result[i] = a[i] + b;
  return result;
}

template <typename T, int N>
KFORCE_INLINE tVectorN<T, N> operator - (tVectorN<T, N> const & a, T const & b)
{
  tVectorN<T, N> result;
  for (int i = 0; i < N; i++) result[i] = a[i] - b;
  return result;
}

template <typename T, int N>
KFORCE_INLINE tVectorN<T, N> operator * (tVectorN<T, N> const & a, tVectorN<T, N> const & b)
{
  tVectorN<T, N> result;
  for ( int i = 0; i < N; i++ ) result[ i ] = a[ i ] * b[ i ];
  return result;
}

template <typename T, int N>
KFORCE_INLINE tVectorN<T, N> operator * (tVectorN<T, N> const & a, typename tVectorN<T, N>::value_type const & b)
{
  tVectorN<T, N> result;
  for ( int i = 0; i < N; i++ )
    result[ i ] = a[ i ] * b;
  return result;
}

template <typename T, int N>
KFORCE_INLINE tVectorN<T, N> operator / (tVectorN<T, N> const & a, typename tVectorN<T, N>::value_type const & b)
{
  tVectorN<T, N> result;
  for ( int i = 0; i < N; i++ )
    result[ i ] = a[ i ] / b;
  return result;
}

// [0] DotProduct------------------------------------------------------------------------------
template <int N, class T>
class tDotProduct {
public:
  static typename T::value_type result( T a, T b ) {
    return a[ N - 1 ] * b[ N - 1 ] + tDotProduct<N - 1, T>::result( a, b );
  }
};
template <class T>
class tDotProduct<1, T> {
public:
  static typename T::value_type result( T a, T b ) {
    return a[ 0 ] * b[ 0 ];
  }
};
template <class T>
KFORCE_INLINE typename T::value_type DotProduct( T a, T b )
{
  return tDotProduct<T::Len, T>::result( a, b );
}


template <typename T>
struct tVectorN<T, 2>
{
  enum { Len = 2 };
  typedef T value_type;
  tVectorN() { m_data[ 0 ] = 0; m_data[ 1 ] = 0; }
  tVectorN( T x, T y ) { m_data[ 0 ] = x; m_data[ 1 ] = y; }
  tVectorN( const T *ptr )	{ this->template init<T>( ptr ); }

  T& operator [] ( int index )				{ assert( index < 2 && "tVector2 : index < 2 -- Failed !" ); return m_data[ index ]; }
  const T operator [] ( int index )const	{ assert( index < 2 && "tVector2 : index < N -- Failed !" ); return m_data[ index ]; }

  const T X() const { return m_data[ 0 ]; }
  const T Y() const { return m_data[ 1 ]; }
  //-------------------------------------------------------
  template <class U>
  void init( U *s_offset )
  {
    assert( sizeof(U) <= sizeof(T) && s_offset );
    ::memcpy( m_data, s_offset, 2 * sizeof(U) );
  }
  //-------------------------------------------------------
private:
  union{
    T m_data[ 2 ];
    struct {
      T x, y;
    };
  };
};

template <typename T>
struct tVectorN<T, 3>
{
  enum { Len = 3 };
  typedef T value_type;

  tVectorN() { m_data[ 0 ] = 0; m_data[ 1 ] = 0; m_data[ 2 ] = 0; }
  tVectorN( T one ) { m_data[ 0 ] = one; m_data[ 1 ] = one; m_data[ 2 ] = one; }
  tVectorN( T x, T y, T z ) { m_data[ 0 ] = x; m_data[ 1 ] = y;  m_data[ 2 ] = z; }
  tVectorN( const T *ptr )	{ this->template init<T>( ptr ); }

  KFORCE_INLINE T& operator [] ( int index )				{ assert( index < 3 && "tVector3 : index < 3 -- Failed !" ); return m_data[ index ]; }
  KFORCE_INLINE const T operator [] ( int index )const	{ assert( index < 3 && "tVector3 : index < 3 -- Failed !" ); return m_data[ index ]; }

  operator const T* ()
  {
    return &m_data[ 0 ];
  }

  //-------------------------------------------------------
  template <class U>
  void init( U *s_offset )
  {
    assert( sizeof(U) <= sizeof(T) && s_offset );
    ::memcpy( m_data, s_offset, 4 * sizeof(U) );
  }
  //-------------------------------------------------------

  KFORCE_INLINE tVectorN & operator += (const tVectorN & other)
  {
    this->x += other.x; this->y += other.y; this->z += other.z;
    return *this;
  }

  KFORCE_INLINE tVectorN & operator -= (const tVectorN & other)
  {
    this->x -= other.x; this->y -= other.y; this->z -= other.z;
    return *this;
  }

  KFORCE_INLINE tVectorN & operator -= (const T & other)
  {
    this->x -= other; this->y -= other; this->z -= other;
    return *this;
  }

// No help for anonymous union but For reading
  union{
    T m_data[ 3 ];
    struct{
      T x, y, z;
    };
  };
};

template <typename T>
tVectorN<T, 3> Normalize( const tVectorN<T, 3> &vec )
{
  /*T len_inv = T( 1.0 ) / T(vec.Length());
  return tVectorN<T, 3>( vec[ 0 ] * len_inv, vec[ 1 ] * len_inv, vec[ 2 ] * len_inv );
  */
  typename tVectorN<T, 3>::value_type sqr = vec.x * vec.x + vec.y * vec.y + vec.z * vec.z;
  return vec * T( 1.0 ) / sqrt( sqr );;
}

template <typename T>
struct tVectorN<T, 4>
{
  enum { Len = 4 };
  typedef T value_type;
  tVectorN() { m_data[ 0 ] = 0; m_data[ 1 ] = 0; m_data[ 2 ] = 0; m_data[ 3 ] = 0; }
  tVectorN( T x, T y, T z, T w ) { m_data[ 0 ] = x; m_data[ 1 ] = y;  m_data[ 2 ] = z; m_data[ 3 ] = w; }
  tVectorN( const tVectorN<T, 3> &vec, T w ) { m_data[ 0 ] = vec[ 0 ]; m_data[ 1 ] = vec[ 1 ]; m_data[ 2 ] = vec[ 2 ]; m_data[ 0 ] = w; }
  tVectorN( const T *ptr )	{ this->template init<T>( ptr ); }

  T& operator [] ( int index )				{ assert( index < 4 && "tVector4 : index < 4 -- Failed !" ); return m_data[ index ]; }
  const T operator [] ( int index )const	{ assert( index < 4 && "tVector4 : index < 4 -- Failed !" ); return m_data[ index ]; }

  operator const T* () const
  {
    return &m_data[ 0 ];
  }

  operator tVectorN<T, 3>() const
  {
    return tVectorN<T, 3>( x, y, z );
  }

  KFORCE_INLINE tVectorN<T, 3> ToVec3() const
  {
      return tVectorN<T, 3>( this->x, this->y, this->z );
  }

  //-------------------------------------------------------
  template <class U>
  KFORCE_INLINE void init( const U *s_offset )
  {
    assert( sizeof(U) <= sizeof(T) && s_offset );
    ::memcpy( m_data, s_offset, 4 * sizeof(U) );
  }

  //-------------------------------------------------------
protected: // No help for anonymous union but For reading
  union{
    T m_data[ 4 ];
    struct{
      T x, y, z, w;
    };
  };
};

//-------------------------------------Matrix Math-------------------------------------------//
template <typename T, int _Col, int _Row>
struct tMatrixMxN{
  typedef T value_type;
  enum { Col = _Col, Row = _Row };
  typedef tVectorN<T, _Col> RowType;

  tMatrixMxN() { ::memset( &data[ 0 ][ 0 ], 0, _Col*_Row*sizeof(T) ); }
  RowType& operator [] ( int index ) { assert( index < _Row && "tMatrixNxN : index < _Row -- Failed !" );    return data[ index ]; }
  const RowType operator [] ( int index ) const { assert( index < _Row && "tMatrixNxN : index < _Row -- Failed !" ); return data[ index ]; }

protected:
  RowType data[ _Row ];
};

template <typename T, int N>
class tMatrixNxN;

template <typename T, int NCol>
class tMatrixNxN{
public:
  typedef T value_type;
  enum { N = NCol };
  typedef tVectorN<T, NCol> RowType;
  KFORCE_INLINE tMatrixNxN() {}
  KFORCE_INLINE tMatrixNxN( const tMatrixNxN& that ) { Assign( that ); }

  KFORCE_INLINE RowType& operator [] ( int index ) { assert( index < N && "tMatrixNxN : index < N -- Failed !" );    return data[ index ]; }
  KFORCE_INLINE const RowType operator [] ( int index ) const { assert( index < NCol && "tMatrixNxN : index < N -- Failed !" ); return data[ index ]; }

//  friend KFORCE_INLINE tMatrixNxN operator * (const tMatrixNxN &a, const tMatrixNxN &b);
//  friend KFORCE_INLINE tMatrixNxN operator * (const tMatrixNxN &a, const RowType &b);
//  friend KFORCE_INLINE tMatrixNxN operator * (const RowType &a, const tMatrixNxN &b);
  friend KFORCE_INLINE tMatrixNxN Transpose( const tMatrixNxN &a ){
      tMatrixNxN<T, NCol> result;
      for ( int i = 0; i < NCol; i++ )
        for ( int j = 0; j < NCol; j++ )
          result[ i ][ j ] = a[ j ][ i ];
      return result;

  }


  operator const T* ()
  {
    return data[ 0 ];
  }

protected:
  KFORCE_INLINE void Assign( const tMatrixNxN& that )
  {
    int n;
    for ( n = 0; n < NCol; n++ )
      data[ n ] = that.data[ n ];
  }

  RowType data[ NCol ];
};

//! \fn	template <typename T, int N> KFORCE_INLINE tMatrixNxN<T, N> operator+ (const tMatrixNxN<T, N> &a, const tMatrixNxN<T, N> &b)
//! \brief	Addition operator.
//! \tparam	T 	Generic type parameter.
//! \tparam	N	Type of the n.
//! \param	a	The const tMatrixNxN&lt;T,N&gt; &amp; to process.
//! \param	b	The const tMatrixNxN&lt;T,N&gt; &amp; to process.
//! \return	The result of the operation.
template <typename T, int N>
KFORCE_INLINE tMatrixNxN<T, N> operator - (const tMatrixNxN<T, N> &a, const tMatrixNxN<T, N> &b)
{
  tMatrixNxN<T, N> result;
  //tMatrixNxN<T, N> bt = Transpose( b );//this line is too expensive
  for (int i = 0; i < N; i++)
  {
    for (int j = 0; j < N; j++)
    {
      result[i][j] += a[i][j] - b[i][j];
    }
  }
  return result;
}

//! \fn	template <typename T, int N> KFORCE_INLINE tMatrixNxN<T, N> operator+ (const tMatrixNxN<T, N> &a, const tMatrixNxN<T, N> &b)
//! \brief	Addition operator of Two Matrices.
//! \tparam	T 	Generic type parameter.
//! \tparam	N	Type of the n.
//! \param	a	The const tMatrixNxN&lt;T,N&gt; &amp; to process.
//! \param	b	The const tMatrixNxN&lt;T,N&gt; &amp; to process.
//! \return	The result of the operation.
template <typename T, int N>
KFORCE_INLINE tMatrixNxN<T, N> operator + (const tMatrixNxN<T, N> &a, const tMatrixNxN<T, N> &b)
{
  tMatrixNxN<T, N> result;
  //tMatrixNxN<T, N> bt = Transpose( b );//this line is too expensive
  for (int i = 0; i < N; i++)
  {
    for (int j = 0; j < N; j++)
    {
      result[i][j] += a[i][j] + b[i][j];
    }
  }
  return result;
}

//! \fn	template <typename T, int N> KFORCE_INLINE tMatrixNxN<T, N> operator* (const tMatrixNxN<T, N> &a, const tMatrixNxN<T, N> &b)
//! \brief	Multiplication of Two tMatrixes.
//! \tparam	T 	Generic type parameter.
//! \tparam	N	Type of the n.
//! \param	a	The const tMatrixNxN&lt;T,N&gt; &amp; to process.
//! \param	b	The const tMatrixNxN&lt;T,N&gt; &amp; to process.
//! \return	The result of the operation.
template <typename T, int N>
KFORCE_INLINE tMatrixNxN<T, N> operator * (const tMatrixNxN<T, N> &a, const tMatrixNxN<T, N> &b)
{
  tMatrixNxN<T, N> result;
  //tMatrixNxN<T, N> bt = Transpose( b );//this line is too expensive
  for ( int i = 0; i < N; i++ )
  {
    for ( int j = 0; j < N; j++ )
    {
      T sum = T( 0.0 );
      for ( int n = 0; n < N; n++ )
        sum += a[ i ][ j ] * b[ j ][ n ];
      //    result[ i ][ j ] = DotProduct( a[ i ], b[ j ] );//meta-programming doesn't optimize much
      result[ i ][ j ] = sum;
    }
  }
  return result;
}

//! \fn	template <typename T, int N> KFORCE_INLINE tVectorN<T, N> operator* (const tMatrixNxN<T, N> &a, const tVectorN<T, N> &b)
//! \brief	Multiplication Of tMatrixNN and tVector.
//! \tparam	T 	Generic type parameter.
//! \tparam	N	Type of the n.
//! \param	a	The const tMatrixNxN&lt;T,N&gt; &amp; to process.
//! \param	b	The const tVectorN&lt;T,N&gt; &amp; to process.
//! \return	The result of the operation.
template <typename T, int N>
KFORCE_INLINE tVectorN<T, N> operator * (const tMatrixNxN<T, N> &a, const tVectorN<T, N> &b)
{
  tVectorN<T, N> result;

  for ( int i = 0; i < N; i++ )
  {
    result[ i ] = DotProduct( a[ i ], b );
  }
  return result;
}

//! \fn	template <typename T, int N> KFORCE_INLINE std::ostream & operator<< (std::ostream & os, const tVectorN<T, N> &vec)
//! \brief	&lt;&lt;&lt;typename T,int N&gt; casting operator.
//! \param [in,out]	os	The operating system.
//! \param	vec					The vector.
//! \return	The result of the operation.
template <typename T, int N>
KFORCE_INLINE std::ostream & operator << (std::ostream & os, const tVectorN<T, N> &vec)
{
  os << "Vector" << N << ":\n";
  for ( int i = 0; i < N; i++ )
  {
    if ( i == 0 )
      os << "\t";
    os << vec[ i ];
    if ( i == N - 1 )
      os << std::endl;
    else
      os << ", ";
  }
  os << std::endl;
  return os;
}

template <typename T, int _Col, int _Row >
KFORCE_INLINE std::ostream & operator << (std::ostream & os, const tMatrixMxN<T, _Col, _Row> &mat)
{
  os << "Mat" << _Col << "x" << _Row << ":\n";
  for ( int i = 0; i < _Row; i++ )
    for ( int j = 0; j < _Col; j++ )
    {
      if ( j == 0 )
        os << "\t";
      os << mat[ i ][ j ];
      if ( j == _Col - 1 )
        os << std::endl;
      else
        os << ", ";
    }
  os << std::endl;
  return os;
}

template <typename T, int N>
KFORCE_INLINE std::ostream & operator << (std::ostream & os, const tMatrixNxN<T, N> &mat)
{
  os << "Mat" << N << ":\n";
  for ( int i = 0; i < N; i++ )
    for ( int j = 0; j < N; j++ )
    {
      if ( j == 0 )
        os << "\t";
      os << mat[ i ][ j ];
      if ( j == N - 1 )
        os << std::endl;
      else
        os << ", ";
    }
  os << std::endl;
  return os;
}

//--------------------------------------------------------------------
template <typename T>
class tMatrixNxN<T, 4>
{
public:
  typedef T value_type;
  enum { N = 4 };
  typedef tVectorN<T, 4> RowType;

  tMatrixNxN<T, 4>() {}

  tMatrixNxN<T,4>( std::initializer_list<T>& list)
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

  tMatrixNxN<T, 4>( RowType &row0, RowType &row1, RowType &row2, RowType &row3 )
  {
    data[ 0 ] = row0; data[ 1 ] = row1; data[ 2 ] = row2; data[ 3 ] = row3;
  }

  //  explicit tMatrixNxN<T, 4>( value_type row[][ 4 ] )
  //  {
  //    data[ 0 ].init( row[ 0 ] );
  //    data[ 1 ].init( row[ 1 ] );
  //    data[ 2 ].init( row[ 2 ] );
  //    data[ 3 ].init( row[ 3 ] );
  //  }

  explicit tMatrixNxN<T, 4>( value_type raw_data[ 16 ] )
  {
    data[ 0 ].init( raw_data );
    data[ 1 ].init( raw_data + 4 );
    data[ 2 ].init( raw_data + 8 );
    data[ 3 ].init( raw_data + 12 );
  }

  RowType& operator [] ( int index ) { assert( index < 4 && "tMatrixNxN : index < N -- Failed !" );    return data[ index ]; }
  const RowType operator [] ( int index ) const { assert( index < 4 && "tMatrixNxN : index < N -- Failed !" ); return data[ index ]; }

  operator const T* () const
  {
    return data[ 0 ];
  }

  friend KFORCE_INLINE tMatrixNxN<T, 4> operator * (const tMatrixNxN<T, 4> & m1, const tMatrixNxN<T, 4> & m2)
  {
    typename tMatrixNxN<T, 4>::RowType const SrcA0 = m1[ 0 ];
    typename tMatrixNxN<T, 4>::RowType const SrcA1 = m1[ 1 ];
    typename tMatrixNxN<T, 4>::RowType const SrcA2 = m1[ 2 ];
    typename tMatrixNxN<T, 4>::RowType const SrcA3 = m1[ 3 ];

    typename tMatrixNxN<T, 4>::RowType const SrcB0 = m2[ 0 ];
    typename tMatrixNxN<T, 4>::RowType const SrcB1 = m2[ 1 ];
    typename tMatrixNxN<T, 4>::RowType const SrcB2 = m2[ 2 ];
    typename tMatrixNxN<T, 4>::RowType const SrcB3 = m2[ 3 ];

    tMatrixNxN<T, 4> Result;
    Result[ 0 ] = SrcA0 * SrcB0[ 0 ] + SrcA1 * SrcB0[ 1 ] + SrcA2 * SrcB0[ 2 ] + SrcA3 * SrcB0[ 3 ];
    Result[ 1 ] = SrcA0 * SrcB1[ 0 ] + SrcA1 * SrcB1[ 1 ] + SrcA2 * SrcB1[ 2 ] + SrcA3 * SrcB1[ 3 ];
    Result[ 2 ] = SrcA0 * SrcB2[ 0 ] + SrcA1 * SrcB2[ 1 ] + SrcA2 * SrcB2[ 2 ] + SrcA3 * SrcB2[ 3 ];
    Result[ 3 ] = SrcA0 * SrcB3[ 0 ] + SrcA1 * SrcB3[ 1 ] + SrcA2 * SrcB3[ 2 ] + SrcA3 * SrcB3[ 3 ];
    return Result;
  }

  KFORCE_INLINE tMatrixNxN<T, 4> Inverse()
  {
    tMatrixNxN<T, 4> result;
    T temp[ 12 ];
    temp[ 0 ] = data[ 2 ][ 2 ] * data[ 3 ][ 3 ]; temp[ 1 ] = data[ 2 ][ 3 ] * data[ 3 ][ 2 ]; temp[ 2 ] = data[ 2 ][ 1 ] * data[ 3 ][ 3 ];
    temp[ 3 ] = data[ 2 ][ 3 ] * data[ 3 ][ 1 ]; temp[ 4 ] = data[ 2 ][ 1 ] * data[ 3 ][ 2 ]; temp[ 5 ] = data[ 2 ][ 2 ] * data[ 3 ][ 1 ];
    temp[ 6 ] = data[ 2 ][ 0 ] * data[ 3 ][ 3 ]; temp[ 7 ] = data[ 2 ][ 3 ] * data[ 3 ][ 0 ]; temp[ 8 ] = data[ 2 ][ 0 ] * data[ 3 ][ 2 ];
    temp[ 9 ] = data[ 2 ][ 2 ] * data[ 3 ][ 0 ]; temp[ 10 ] = data[ 2 ][ 0 ] * data[ 3 ][ 1 ]; temp[ 11 ] = data[ 2 ][ 1 ] * data[ 3 ][ 0 ];
    result[ 0 ][ 0 ] = temp[ 0 ] * data[ 1 ][ 1 ] + temp[ 3 ] * data[ 1 ][ 2 ] + temp[ 4 ] * data[ 1 ][ 3 ] - (temp[ 1 ] * data[ 1 ][ 1 ] + temp[ 2 ] * data[ 1 ][ 2 ] + temp[ 5 ] * data[ 1 ][ 3 ]);
    result[ 1 ][ 0 ] = temp[ 1 ] * data[ 1 ][ 0 ] + temp[ 6 ] * data[ 1 ][ 2 ] + temp[ 9 ] * data[ 1 ][ 3 ] - (temp[ 0 ] * data[ 1 ][ 0 ] + temp[ 7 ] * data[ 1 ][ 2 ] + temp[ 8 ] * data[ 1 ][ 3 ]);
    result[ 2 ][ 0 ] = temp[ 2 ] * data[ 1 ][ 0 ] + temp[ 7 ] * data[ 1 ][ 1 ] + temp[ 10 ] * data[ 1 ][ 3 ] - (temp[ 3 ] * data[ 1 ][ 0 ] + temp[ 6 ] * data[ 1 ][ 1 ] + temp[ 11 ] * data[ 1 ][ 3 ]);
    result[ 3 ][ 0 ] = temp[ 5 ] * data[ 1 ][ 0 ] + temp[ 8 ] * data[ 1 ][ 1 ] + temp[ 11 ] * data[ 1 ][ 2 ] - (temp[ 4 ] * data[ 1 ][ 0 ] + temp[ 9 ] * data[ 1 ][ 1 ] + temp[ 10 ] * data[ 1 ][ 2 ]);
    result[ 0 ][ 1 ] = temp[ 1 ] * data[ 0 ][ 1 ] + temp[ 2 ] * data[ 0 ][ 2 ] + temp[ 5 ] * data[ 0 ][ 3 ] - (temp[ 0 ] * data[ 0 ][ 1 ] + temp[ 3 ] * data[ 0 ][ 2 ] + temp[ 4 ] * data[ 0 ][ 3 ]);
    result[ 1 ][ 1 ] = temp[ 0 ] * data[ 0 ][ 0 ] + temp[ 7 ] * data[ 0 ][ 2 ] + temp[ 8 ] * data[ 0 ][ 3 ] - (temp[ 1 ] * data[ 0 ][ 0 ] + temp[ 6 ] * data[ 0 ][ 2 ] + temp[ 9 ] * data[ 0 ][ 3 ]);
    result[ 2 ][ 1 ] = temp[ 3 ] * data[ 0 ][ 0 ] + temp[ 6 ] * data[ 0 ][ 1 ] + temp[ 11 ] * data[ 0 ][ 3 ] - (temp[ 2 ] * data[ 0 ][ 0 ] + temp[ 7 ] * data[ 0 ][ 1 ] + temp[ 10 ] * data[ 0 ][ 3 ]);
    result[ 3 ][ 1 ] = temp[ 4 ] * data[ 0 ][ 0 ] + temp[ 9 ] * data[ 0 ][ 1 ] + temp[ 10 ] * data[ 0 ][ 2 ] - (temp[ 5 ] * data[ 0 ][ 0 ] + temp[ 8 ] * data[ 0 ][ 1 ] + temp[ 11 ] * data[ 0 ][ 2 ]);
    temp[ 0 ] = data[ 0 ][ 2 ] * data[ 1 ][ 3 ]; temp[ 1 ] = data[ 0 ][ 3 ] * data[ 1 ][ 2 ]; temp[ 2 ] = data[ 0 ][ 1 ] * data[ 1 ][ 3 ]; temp[ 3 ] = data[ 0 ][ 3 ] * data[ 1 ][ 1 ];
    temp[ 4 ] = data[ 0 ][ 1 ] * data[ 1 ][ 2 ]; temp[ 5 ] = data[ 0 ][ 2 ] * data[ 1 ][ 1 ]; temp[ 6 ] = data[ 0 ][ 0 ] * data[ 1 ][ 3 ]; temp[ 7 ] = data[ 0 ][ 3 ] * data[ 1 ][ 0 ];
    temp[ 8 ] = data[ 0 ][ 0 ] * data[ 1 ][ 2 ]; temp[ 9 ] = data[ 0 ][ 2 ] * data[ 1 ][ 0 ]; temp[ 10 ] = data[ 0 ][ 0 ] * data[ 1 ][ 1 ]; temp[ 11 ] = data[ 0 ][ 1 ] * data[ 1 ][ 0 ];
    result[ 0 ][ 2 ] = temp[ 0 ] * data[ 3 ][ 1 ] + temp[ 3 ] * data[ 3 ][ 2 ] + temp[ 4 ] * data[ 3 ][ 3 ] - (temp[ 1 ] * data[ 3 ][ 1 ] + temp[ 2 ] * data[ 3 ][ 2 ] + temp[ 5 ] * data[ 3 ][ 3 ]);
    result[ 1 ][ 2 ] = temp[ 1 ] * data[ 3 ][ 0 ] + temp[ 6 ] * data[ 3 ][ 2 ] + temp[ 9 ] * data[ 3 ][ 3 ] - (temp[ 0 ] * data[ 3 ][ 0 ] + temp[ 7 ] * data[ 3 ][ 2 ] + temp[ 8 ] * data[ 3 ][ 3 ]);
    result[ 2 ][ 2 ] = temp[ 2 ] * data[ 3 ][ 0 ] + temp[ 7 ] * data[ 3 ][ 1 ] + temp[ 10 ] * data[ 3 ][ 3 ] - (temp[ 3 ] * data[ 3 ][ 0 ] + temp[ 6 ] * data[ 3 ][ 1 ] + temp[ 11 ] * data[ 3 ][ 3 ]);
    result[ 3 ][ 2 ] = temp[ 5 ] * data[ 3 ][ 0 ] + temp[ 8 ] * data[ 3 ][ 1 ] + temp[ 11 ] * data[ 3 ][ 2 ] - (temp[ 4 ] * data[ 3 ][ 0 ] + temp[ 9 ] * data[ 3 ][ 1 ] + temp[ 10 ] * data[ 3 ][ 2 ]);
    result[ 0 ][ 3 ] = temp[ 2 ] * data[ 2 ][ 2 ] + temp[ 5 ] * data[ 2 ][ 3 ] + temp[ 1 ] * data[ 2 ][ 1 ] - (temp[ 4 ] * data[ 2 ][ 3 ] + temp[ 0 ] * data[ 2 ][ 1 ] + temp[ 3 ] * data[ 2 ][ 2 ]);
    result[ 1 ][ 3 ] = temp[ 8 ] * data[ 2 ][ 3 ] + temp[ 0 ] * data[ 2 ][ 0 ] + temp[ 7 ] * data[ 2 ][ 2 ] - (temp[ 6 ] * data[ 2 ][ 2 ] + temp[ 9 ] * data[ 2 ][ 3 ] + temp[ 1 ] * data[ 2 ][ 0 ]);
    result[ 2 ][ 3 ] = temp[ 6 ] * data[ 2 ][ 1 ] + temp[ 11 ] * data[ 2 ][ 3 ] + temp[ 3 ] * data[ 2 ][ 0 ] - (temp[ 10 ] * data[ 2 ][ 3 ] + temp[ 2 ] * data[ 2 ][ 0 ] + temp[ 7 ] * data[ 2 ][ 1 ]);
    result[ 3 ][ 3 ] = temp[ 10 ] * data[ 2 ][ 2 ] + temp[ 4 ] * data[ 2 ][ 0 ] + temp[ 9 ] * data[ 2 ][ 1 ] - (temp[ 8 ] * data[ 2 ][ 1 ] + temp[ 11 ] * data[ 2 ][ 2 ] + temp[ 5 ] * data[ 2 ][ 0 ]);

    T idet = T( 1.0 ) / (data[ 0 ][ 0 ] * result[ 0 ][ 0 ] + data[ 0 ][ 1 ] * result[ 1 ][ 0 ] + data[ 0 ][ 2 ] * result[ 2 ][ 0 ] + data[ 0 ][ 3 ] * result[ 3 ][ 0 ]);

    result[ 0 ][ 0 ] *= idet; result[ 0 ][ 1 ] *= idet; result[ 0 ][ 2 ] *= idet; result[ 0 ][ 3 ] *= idet;
    result[ 1 ][ 0 ] *= idet; result[ 1 ][ 1 ] *= idet; result[ 1 ][ 2 ] *= idet; result[ 1 ][ 3 ] *= idet;
    result[ 2 ][ 0 ] *= idet; result[ 2 ][ 1 ] *= idet; result[ 2 ][ 2 ] *= idet; result[ 2 ][ 3 ] *= idet;
    result[ 3 ][ 0 ] *= idet; result[ 3 ][ 1 ] *= idet; result[ 3 ][ 2 ] *= idet; result[ 3 ][ 3 ] *= idet;

    return result;
  }

protected:
  RowType data[ 4 ];
};

//---------------------------------------------------------
template <typename T>
KFORCE_INLINE T ToRadian( T const & degree )
{
  assert( kType<T>::is_float );

  T const pi = T( 3.1415926535897932384626433832795 );
  return degree * (pi / T( 180 ));
}

template <typename T>
KFORCE_INLINE T ToDegree( T const & rad )
{
  assert( kType<T>::is_float );

  const T pi = T( 3.1415926535897932384626433832795 );
  return rad * (T( 180 ) / pi);
}
//-----------------------------------------------------------

template <typename T>
static KFORCE_INLINE tVectorN<T, 3> CrossProduct( const tVectorN<T, 3>& a, const tVectorN<T, 3>& b )
{
  return tVectorN<T, 3>( a[ 1 ] * b[ 2 ] - b[ 1 ] * a[ 2 ],
      a[ 2 ] * b[ 0 ] - b[ 2 ] * a[ 0 ],
      a[ 0 ] * b[ 1 ] - b[ 0 ] * a[ 1 ] );
}

template <typename T>
KFORCE_INLINE tMatrixNxN<T, 4> MakeIdentityMatrix()
{
  static tMatrixNxN<T, 4> one;
  one[ 0 ][ 0 ] = T( 1.0 );
  one[ 1 ][ 1 ] = T( 1.0 );
  one[ 2 ][ 2 ] = T( 1.0 );
  one[ 3 ][ 3 ] = T( 1.0 );
  return one;
}

template <typename T>
KFORCE_INLINE tMatrixNxN<T, 4> Rotate( const tVectorN<T, 3> &vec, T angle, const tMatrixNxN<T, 4>& m )
{
  T rad = ToRadian( angle );
  T c = cos( rad );
  T s = sin( rad );

  tVectorN<T, 3> axis = Normalize( vec );
  tVectorN<T, 3> temp = axis * (T( 1 ) - c);

  tMatrixNxN<T, 4> Rotate;
  Rotate[ 0 ][ 0 ] = c + temp[ 0 ] * axis[ 0 ];
  Rotate[ 0 ][ 1 ] = 0 + temp[ 0 ] * axis[ 1 ] + s * axis[ 2 ];
  Rotate[ 0 ][ 2 ] = 0 + temp[ 0 ] * axis[ 2 ] - s * axis[ 1 ];

  Rotate[ 1 ][ 0 ] = 0 + temp[ 1 ] * axis[ 0 ] - s * axis[ 2 ];
  Rotate[ 1 ][ 1 ] = c + temp[ 1 ] * axis[ 1 ];
  Rotate[ 1 ][ 2 ] = 0 + temp[ 1 ] * axis[ 2 ] + s * axis[ 0 ];

  Rotate[ 2 ][ 0 ] = 0 + temp[ 2 ] * axis[ 0 ] + s * axis[ 1 ];
  Rotate[ 2 ][ 1 ] = 0 + temp[ 2 ] * axis[ 1 ] - s * axis[ 0 ];
  Rotate[ 2 ][ 2 ] = c + temp[ 2 ] * axis[ 2 ];

  tMatrixNxN<T, 4> Result;
  Result[ 0 ] = m[ 0 ] * Rotate[ 0 ][ 0 ] + m[ 1 ] * Rotate[ 0 ][ 1 ] + m[ 2 ] * Rotate[ 0 ][ 2 ];
  Result[ 1 ] = m[ 0 ] * Rotate[ 1 ][ 0 ] + m[ 1 ] * Rotate[ 1 ][ 1 ] + m[ 2 ] * Rotate[ 1 ][ 2 ];
  Result[ 2 ] = m[ 0 ] * Rotate[ 2 ][ 0 ] + m[ 1 ] * Rotate[ 2 ][ 1 ] + m[ 2 ] * Rotate[ 2 ][ 2 ];
  Result[ 3 ] = m[ 3 ];
  return Result;
}

/// Make A Rotation Matrix
/// \brief MakeRotationMatrix
/// \param vec
/// \param angle
/// \return Origin Matrix Multiplied Rotation Matrix
template <typename T>
KFORCE_INLINE tMatrixNxN<T, 4> MakeRotationMatrix( const tVectorN<T, 3> &vec, T angle )
{
  return Rotate( vec, angle, MakeIdentityMatrix<T>() );
}

/// Translate The Matrix "m" by Vector v
/// \brief Translate
/// \param v
/// \param m
/// \return Translated Matrix
template <typename T>
KFORCE_INLINE tMatrixNxN<T, 4> Translate( const tVectorN<T, 3> &v, const tMatrixNxN<T, 4>& m )
{
  tMatrixNxN<T, 4> res( m );
  res[ 3 ] = m[ 0 ] * v[ 0 ] + m[ 1 ] * v[ 1 ] + m[ 2 ] * v[ 2 ] + m[ 3 ];
  return res;
}

///
/// \brief MakeTranslationMatrix
/// \param v Translation Vector 3
/// \return translated Matrix
template <typename T>
KFORCE_INLINE tMatrixNxN<T, 4> MakeTranslationMatrix( T x, T y, T z )
{
  tMatrixNxN<T,4> mat4;
  mat4[0] = {T( 1.0 ), T( 0.0 ), T( 0.0 ), T( 0.0 )};
  mat4[1] = {T( 0.0 ), T( 1.0 ), T( 0.0 ), T( 0.0 )};
  mat4[2] = {T( 0.0 ), T( 0.0 ), T( 1.0 ), T( 0.0 )};
  mat4[3] = {x, y, z, T( 1.0 )};

  return mat4;
}

///
/// \brief MakeTranslationMatrix
/// \param v Translation Vector 3
/// \return translated Matrix
template <typename T>
KFORCE_INLINE tMatrixNxN<T, 4> MakeTranslationMatrix( tVectorN<T, 3> & vec )
{
  return MakeTranslationMatrix( vec[ 0 ], vec[ 1 ], vec[ 2 ] );
}

//! \fn template <typename T> KFORCE_INLINE tMatrixNxN<T, 4> MakeFrustum( T left, T right, T bottom, T top, T n, T f )
//! \brief  Makes a frustum matrix For Light...
//! \tparam T can be float, double.
//! \param  left    The left plane distance.
//! \param  right   The right plane distance.
//! \param  bottom  The bottom plane distance.
//! \param  top     The top plane distance.
//! \param  n       near plane distance
//! \param  f       far plane distance
//!
//! \return A Frustum tMatrixNxN&lt;T,4&gt;
template <typename T>
KFORCE_INLINE tMatrixNxN<T, 4> MakeFrustum(
    T left, T right,
    T bottom, T top,
    T n, T f )
{
  tMatrixNxN<T, 4> result( MakeIdentityMatrix<T>() );

  if ( (right == left) ||
       (top == bottom) ||
       (n == f) ||
       (n < T( 0.0 )) ||
       (f < T( 0.0 )) )
    return result;

  result[ 0 ][ 0 ] = (T( 2.0 ) * n) / (right - left);
  result[ 1 ][ 1 ] = (T( 2.0 ) * n) / (top - bottom);

  result[ 2 ][ 0 ] = (right + left) / (right - left);
  result[ 2 ][ 1 ] = (top + bottom) / (top - bottom);
  result[ 2 ][ 2 ] = -(f + n) / (f - n);
  result[ 2 ][ 3 ] = -T( 1.0 );

  result[ 3 ][ 2 ] = -(T( 2.0 ) * f * n) / (f - n);
  result[ 3 ][ 3 ] = T( 0.0 );

  return result;
}

/// template function Perspective
/// \brief Perspective
/// \param fov in Y
/// \param ratio
/// \param near_plane
/// \param far_plane
/// \return Matrix4X4 Perspective Matrix
template <typename T, class MatType = tMatrixNxN<T, 4> >
KFORCE_INLINE MatType Perspective( T fovY, T ratio, T near_plane, T far_plane )
{
  MatType result;
  T q = T( 1.0 ) / T( ::tan( ToRadian( T( 0.5 ) * fovY ) ) );
  T A = q / ratio;
  T B = (near_plane + far_plane) / (near_plane - far_plane);
  T C = (T( 2.0 ) * near_plane * far_plane) / (near_plane - far_plane);
  result[ 0 ] = { A, T( 0.0 ), T( 0.0 ), T( 0.0 ) };
  result[ 1 ] = { T( 0.0 ), q, T( 0.0 ), T( 0.0 ) };
  result[ 2 ] = { T( 0.0 ), T( 0.0 ), B, T( -1.0 ) };
  result[ 3 ] = { T( 0.0 ), T( 0.0 ), C, T( 0.0 ) };
  return result;
}

//! \fn template < typename T > KFORCE_INLINE tMatrixNxN<T, 4> LookAt( const tVectorN<T, 3> & eye, const tVectorN<T, 3> & center, const tVectorN<T, 3> & up )
//! \brief  LookAt for Camera
//! \tparam T Generic type parameter.
//! \param  eye     The eye.
//! \param  center  The center.
//! \param  up      The up.
//!
//! \return A tMatrixNxN&lt;T,4&gt;
template < typename T >
KFORCE_INLINE tMatrixNxN<T, 4> LookAt(
    const tVectorN<T, 3> & eye,
    const tVectorN<T, 3> & center,
    const tVectorN<T, 3> & up )
{
  tVectorN<T, 3> f = Normalize( center - eye );
  tVectorN<T, 3> u = Normalize( up );
  tVectorN<T, 3> s = Normalize( CrossProduct( f, u ) );
  u = CrossProduct( s, f );

  tMatrixNxN<T, 4> Result( MakeIdentityMatrix<T>() );
  Result[ 0 ][ 0 ] = s[ 0 ];
  Result[ 1 ][ 0 ] = s[ 1 ];
  Result[ 2 ][ 0 ] = s[ 2 ];
  Result[ 0 ][ 1 ] = u[ 0 ];
  Result[ 1 ][ 1 ] = u[ 1 ];
  Result[ 2 ][ 1 ] = u[ 2 ];
  Result[ 0 ][ 2 ] = -f[ 0 ];
  Result[ 1 ][ 2 ] = -f[ 1 ];
  Result[ 2 ][ 2 ] = -f[ 2 ];
  Result[ 3 ][ 0 ] = -DotProduct( s, eye );
  Result[ 3 ][ 1 ] = -DotProduct( u, eye );
  Result[ 3 ][ 2 ] = DotProduct( f, eye );
  return Result;
}

//-----------------------------------------------------
//----------------------Matrix Math End----------------------------//
//////////////////////////////////////////////////////////////////////////
//  This is Quaternion Math Part

//! \class  Quaternion
//! \tparam T Generic data type parameter.
template <typename T>
class Quaternion
{
public:
  typedef T value_type;

  Quaternion() {}

  Quaternion( const T axis_angle, const tVectorN<T, 3> & axis )
  {
    m_data[ 0 ] = T( ::cos( ToRadian( axis_angle ) / 2.0 ) );
    m_data[ 1 ] = axis[ 0 ] * T( ::sin( axis_angle / 2.0 ) );
    m_data[ 2 ] = axis[ 1 ] * T( ::sin( axis_angle / 2.0 ) );
    m_data[ 3 ] = axis[ 2 ] * T( ::sin( axis_angle / 2.0 ) );
  }

  explicit Quaternion( const tVectorN<T, 4> & vec4 )
  {
    SetValue( vec4 );
  }

  Quaternion( T _w, T _x, T _y, T _z )
    : w( _w ), x( _x ), y( _y ), z( _z )
  {
  }

  KFORCE_INLINE void SetValue( const tVectorN<T, 4> & vec )
  {
    m_data[ 0 ] = vec[ 0 ];
    m_data[ 1 ] = vec[ 1 ];
    m_data[ 2 ] = vec[ 2 ];
    m_data[ 3 ] = vec[ 3 ];
  }

  /**
   * \fn  KFORCE_INLINE Quaternion & operator*= (const Quaternion & other)
   * \brief Grassmann Product
   * \param [in] another quaternion.
   * \return  The result of the operation.
   */
  KFORCE_INLINE Quaternion & operator *= (const Quaternion & p)
  {
    T t1 = w * p.w - x * p.x - y * p.y - z * p.z;
    T t2 = w * p.x + x * p.w + y * p.z - z * p.y;
    T t3 = w * p.y + y * p.w + z * p.x - x * p.z;
    T t4 = w * p.z + z * p.w + x * p.y - y * p.x;
    this->x = t1; this->y = t2; this->z = t3; this->z = t4;
    return *this;
  }

  //! \fn KFORCE_INLINE Quaternion Conjugate()
  //! \brief  Gets the conjugate.
  //! \return A Quaternion.
  KFORCE_INLINE Quaternion Conjugate() { return Quaternion( w, -x, -y, -z ); }

  //! \fn Quaternion Inverse()
  //! \brief  Gets the inverse.
  //! \return A Quaternion.
  Quaternion Inverse() { return Conjugate() / DotProduct( *this, *this ); }

  //! \fn tMatrixNxN<T, 4> Rotate( T axis_angle, const tVectorN<T,3> & axis );
  //! \brief  Rotates quaternion with axis by angle
  //! \param  axis_angle  The axis angle float in degrees.
  //! \param  axis        The axis Vec3f.
  //! \return A tMatrixNxN&lt;T,4&gt;
  tMatrixNxN<T, 4> Rotate( T axis_angle, const tVectorN<T, 3> & axis )
  {
    tVectorN<T, 3> Tmp = axis;
    typename Quaternion<T>::value_type len = Tmp.Length();
    if ( abs( len - T( 1 ) ) > T( 0.001 ) )
    {
      T oneOverLen = T( 1 ) / len;
      Tmp.x *= oneOverLen;
      Tmp.y *= oneOverLen;
      Tmp.z *= oneOverLen;
    }
    typename Quaternion<T>::value_type const Sin = ::sin( ToRadian( axis_angle ) * T( 0.5 ) );

    return  (*this) * Quaternion<T>( cos( axis_angle * T( 0.5 ) ), Tmp.x * Sin, Tmp.y * Sin, Tmp.z * Sin );
  }

  //! \fn tVectorN<T, 3> ToEulerAngles()
  //! \brief  Converts Quaternion to an euler angles.
  //! \return Euler Angles as Vec3;
  tVectorN<T, 3> ToEulerAngles()
  {
    T _pitch = ToDegree( ::atan2( value_type( 2 ) * (y * z + w * x), w * w - x * x - y * y + z * z ) );
    T _yaw = ToDegree( ::asin( value_type( -2 ) * (x * z - w * y) ) );
    T _roll = ToDegree( ::atan2( value_type( 2 ) * (x * y + w * z), w * w + x * x - y * y - z * z ) );
    return tVectorN<T, 3>( _pitch, _yaw, _roll );
  }

  //! \fn tMatrixNxN<T, 4> AsMatrix();
  //! \brief  Converts this Quaternion to a Mat4f.
  //! \return A tMatrixNxN&lt;T,4&gt;
  KFORCE_INLINE tMatrixNxN<T, 4> AsMatrix()
  {
    tMatrixNxN<T, 4> Result;
    Result[ 0 ][ 0 ] = T(1) - 2 * y * y - 2 * z * z;
    Result[ 0 ][ 1 ] = T(2) * x * y + 2 * w * z;
    Result[ 0 ][ 2 ] = T(2) * x * z - 2 * w * y;

    Result[ 1 ][ 0 ] = T(2) * x * y - 2 * w * z;
    Result[ 1 ][ 1 ] = T(1) - T(2) * x * x - 2 * z * z;
    Result[ 1 ][ 2 ] = T(2) * y * z + 2 * w * x;

    Result[ 2 ][ 0 ] = T(2) * x * z + 2 * w * y;
    Result[ 2 ][ 1 ] = T(2) * y * z - 2 * w * x;
    Result[ 2 ][ 2 ] = T(1) - 2 * x * x - 2 * y * y;

    Result[ 3 ][ 3 ] = T( 1 );
    return Result;
  }

  operator const tMatrixNxN<T, 4>() const
  {
    return AsMatrix();
  }

  static tMatrixNxN<T, 4> YawPitchRoll(T yaw, T pitch, T roll)
  {
    T tmp_ch = ::cos(yaw);
    T tmp_sh = ::sin(yaw);
    T tmp_cp = ::cos(pitch);
    T tmp_sp = ::sin(pitch);
    T tmp_cb = ::cos(roll);
    T tmp_sb = ::sin(roll);

    tMatrixNxN<T, 4> Result;
    Result[0][0] = tmp_ch * tmp_cb + tmp_sh * tmp_sp * tmp_sb;
    Result[0][1] = tmp_sb * tmp_cp;
    Result[0][2] = -tmp_sh * tmp_cb + tmp_ch * tmp_sp * tmp_sb;
    Result[1][0] = -tmp_ch * tmp_sb + tmp_sh * tmp_sp * tmp_cb;
    Result[1][1] = tmp_cb * tmp_cp;
    Result[1][2] = tmp_sb * tmp_sh + tmp_ch * tmp_sp * tmp_cb;
    Result[2][0] = tmp_sh * tmp_cp;
    Result[2][1] = -tmp_sp;
    Result[2][2] = tmp_ch * tmp_cp;
    Result[3][3] = T(1);
    return Result;
  }

  static Quaternion Slerp( const Quaternion & p, const Quaternion &q, value_type t)
  {
    value_type theta = q.w - p.w;
    value_type factor1 = ::sin(value_type(1.0-t)*theta);
    value_type factor2 = ::sin(t*theta);
    value_type factor3 = ::sin(theta);
    return Quaternion((factor1*p.w + factor2*q.w)/factor3, (factor1*p.x + factor2*q.x)/factor3, (factor1*p.y + factor2*q.y)/factor3, (factor1*p.z + factor2*q.z)/factor3 );
  }

private:
  union{
    T m_data[ 4 ];
    struct{
      T w, x, y, z;
    };
  };
};
//--------------------Quaternion Operations With Matrix Math-------------------//

//--------------------CODE BELOW IS FROM MINIENGINE----------------------------//
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Developed by Minigraph
//
// Author:  James Stanard 
//

template <typename T> KFORCE_INLINE T AlignUpWithMask(T value, size_t mask)
{
	return (T)(((size_t)value + mask) & ~mask);
}

template <typename T> KFORCE_INLINE T AlignDownWithMask(T value, size_t mask)
{
	return (T)((size_t)value & ~mask);
}

template <typename T> KFORCE_INLINE T AlignUp(T value, size_t alignment)
{
	return AlignUpWithMask(value, alignment - 1);
}

template <typename T> KFORCE_INLINE T AlignDown(T value, size_t alignment)
{
	return AlignDownWithMask(value, alignment - 1);
}

template <typename T> KFORCE_INLINE bool IsAligned(T value, size_t alignment)
{
	return 0 == ((size_t)value & (alignment - 1));
}

template <typename T> KFORCE_INLINE T DivideByMultiple(T value, size_t alignment)
{
	return (T)((value + alignment - 1) / alignment);
}

template< class T >
static KFORCE_INLINE T Clamp(const T X, const T Min, const T Max)
{
	return X<Min ? Min : X<Max ? X : Max;
}

NS_MATHLIB_END

#endif
