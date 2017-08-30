#pragma once

#ifndef __k3d_Matrix_h__
#define __k3d_Matrix_h__

#include "Vector.h"

namespace k3d
{
    namespace math
    {
        template <typename T, int _Col, int _Row>
        struct TMatrixMN {
            typedef T value_type;
            enum { Col = _Col, Row = _Row };
            typedef TVector<T, _Col> RowType;

            TMatrixMN() { MemoryFill(&data[0][0], 0, _Col*_Row * sizeof(T)); }
            RowType& operator [] (int index) { assert(index < _Row && "TMatrixN : index < _Row -- Failed !");    return data[index]; }
            const RowType operator [] (int index) const { assert(index < _Row && "TMatrixN : index < _Row -- Failed !"); return data[index]; }

        protected:
            RowType data[_Row];
        };

        template <typename T, int N>
        class TMatrixN;

        template <typename T, int NCol>
        class TMatrixN {
        public:
            typedef T value_type;
            enum { N = NCol };
            typedef TVector<T, NCol> RowType;
            KFORCE_INLINE TMatrixN() {}
            KFORCE_INLINE TMatrixN(const TMatrixN& that) { Assign(that); }

            KFORCE_INLINE RowType& operator [] (int index) { assert(index < N && "TMatrixN : index < N -- Failed !");    return data[index]; }
            KFORCE_INLINE const RowType operator [] (int index) const { assert(index < NCol && "TMatrixN : index < N -- Failed !"); return data[index]; }

            //  friend KFORCE_INLINE TMatrixN operator * (const TMatrixN &a, const TMatrixN &b);
            //  friend KFORCE_INLINE TMatrixN operator * (const TMatrixN &a, const RowType &b);
            //  friend KFORCE_INLINE TMatrixN operator * (const RowType &a, const TMatrixN &b);
            friend KFORCE_INLINE TMatrixN Transpose(const TMatrixN &a) {
                TMatrixN<T, NCol> result;
                for (int i = 0; i < NCol; i++)
                    for (int j = 0; j < NCol; j++)
                        result[i][j] = a[j][i];
                return result;

            }


            operator const T* ()
            {
                return data[0];
            }

        protected:
            KFORCE_INLINE void Assign(const TMatrixN& that)
            {
                int n;
                for (n = 0; n < NCol; n++)
                    data[n] = that.data[n];
            }

            RowType data[NCol];
        };

        //! \fn	template <typename T, int N> KFORCE_INLINE TMatrixN<T, N> operator+ (const TMatrixN<T, N> &a, const TMatrixN<T, N> &b)
        //! \brief	Addition operator.
        //! \tparam	T 	Generic type parameter.
        //! \tparam	N	Type of the n.
        //! \param	a	The const TMatrixN&lt;T,N&gt; &amp; to process.
        //! \param	b	The const TMatrixN&lt;T,N&gt; &amp; to process.
        //! \return	The result of the operation.
        template <typename T, int N>
        KFORCE_INLINE TMatrixN<T, N> operator - (const TMatrixN<T, N> &a, const TMatrixN<T, N> &b)
        {
            TMatrixN<T, N> result;
            //TMatrixN<T, N> bt = Transpose( b );//this line is too expensive
            for (int i = 0; i < N; i++)
            {
                for (int j = 0; j < N; j++)
                {
                    result[i][j] += a[i][j] - b[i][j];
                }
            }
            return result;
        }

        //! \fn	template <typename T, int N> KFORCE_INLINE TMatrixN<T, N> operator+ (const TMatrixN<T, N> &a, const TMatrixN<T, N> &b)
        //! \brief	Addition operator of Two Matrices.
        //! \tparam	T 	Generic type parameter.
        //! \tparam	N	Type of the n.
        //! \param	a	The const TMatrixN&lt;T,N&gt; &amp; to process.
        //! \param	b	The const TMatrixN&lt;T,N&gt; &amp; to process.
        //! \return	The result of the operation.
        template <typename T, int N>
        KFORCE_INLINE TMatrixN<T, N> operator + (const TMatrixN<T, N> &a, const TMatrixN<T, N> &b)
        {
            TMatrixN<T, N> result;
            //TMatrixN<T, N> bt = Transpose( b );//this line is too expensive
            for (int i = 0; i < N; i++)
            {
                for (int j = 0; j < N; j++)
                {
                    result[i][j] += a[i][j] + b[i][j];
                }
            }
            return result;
        }

        //! \fn	template <typename T, int N> KFORCE_INLINE TMatrixN<T, N> operator* (const TMatrixN<T, N> &a, const TMatrixN<T, N> &b)
        //! \brief	Multiplication of Two tMatrixes.
        //! \tparam	T 	Generic type parameter.
        //! \tparam	N	Type of the n.
        //! \param	a	The const TMatrixN&lt;T,N&gt; &amp; to process.
        //! \param	b	The const TMatrixN&lt;T,N&gt; &amp; to process.
        //! \return	The result of the operation.
        template <typename T, int N>
        KFORCE_INLINE TMatrixN<T, N> operator * (const TMatrixN<T, N> &a, const TMatrixN<T, N> &b)
        {
            TMatrixN<T, N> result;
            //TMatrixN<T, N> bt = Transpose( b );//this line is too expensive
            for (int i = 0; i < N; i++)
            {
                for (int j = 0; j < N; j++)
                {
                    T sum = T(0.0);
                    for (int n = 0; n < N; n++)
                        sum += a[i][j] * b[j][n];
                    //    result[ i ][ j ] = DotProduct( a[ i ], b[ j ] );//meta-programming doesn't optimize much
                    result[i][j] = sum;
                }
            }
            return result;
        }

        //! \fn	template <typename T, int N> KFORCE_INLINE TVector<T, N> operator* (const TMatrixN<T, N> &a, const TVector<T, N> &b)
        //! \brief	Multiplication Of tMatrixNN and tVector.
        //! \tparam	T 	Generic type parameter.
        //! \tparam	N	Type of the n.
        //! \param	a	The const TMatrixN&lt;T,N&gt; &amp; to process.
        //! \param	b	The const TVector&lt;T,N&gt; &amp; to process.
        //! \return	The result of the operation.
        template <typename T, int N>
        KFORCE_INLINE TVector<T, N> operator * (const TMatrixN<T, N> &a, const TVector<T, N> &b)
        {
            TVector<T, N> result;

            for (int i = 0; i < N; i++)
            {
                result[i] = DotProduct(a[i], b);
            }
            return result;
        }

#ifdef _IOSFWD_
        //! \fn	template <typename T, int N> KFORCE_INLINE std::ostream & operator<< (std::ostream & os, const TVector<T, N> &vec)
        //! \brief	&lt;&lt;&lt;typename T,int N&gt; casting operator.
        //! \param [in,out]	os	The operating system.
        //! \param	vec					The vector.
        //! \return	The result of the operation.
        template <typename T, int N>
        KFORCE_INLINE std::ostream & operator << (std::ostream & os, const TVector<T, N> &vec)
        {
            os << "Vector" << N << ":\n";
            for (int i = 0; i < N; i++)
            {
                if (i == 0)
                    os << "\t";
                os << vec[i];
                if (i == N - 1)
                    os << std::endl;
                else
                    os << ", ";
            }
            os << std::endl;
            return os;
        }

        template <typename T, int _Col, int _Row >
        KFORCE_INLINE std::ostream & operator << (std::ostream & os, const TMatrixMN<T, _Col, _Row> &mat)
        {
            os << "Mat" << _Col << "x" << _Row << ":\n";
            for (int i = 0; i < _Row; i++)
                for (int j = 0; j < _Col; j++)
                {
                    if (j == 0)
                        os << "\t";
                    os << mat[i][j];
                    if (j == _Col - 1)
                        os << std::endl;
                    else
                        os << ", ";
                }
            os << std::endl;
            return os;
        }

        template <typename T, int N>
        KFORCE_INLINE std::ostream & operator << (std::ostream & os, const TMatrixN<T, N> &mat)
        {
            os << "Mat" << N << ":\n";
            for (int i = 0; i < N; i++)
                for (int j = 0; j < N; j++)
                {
                    if (j == 0)
                        os << "\t";
                    os << mat[i][j];
                    if (j == N - 1)
                        os << std::endl;
                    else
                        os << ", ";
                }
            os << std::endl;
            return os;
        }
#endif
        //--------------------------------------------------------------------
        template <typename T>
        class TMatrixN<T, 4>
        {
        public:
            typedef T value_type;
            enum { N = 4 };
            typedef TVector<T, 4> RowType;

            TMatrixN<T, 4>() {}
#ifdef _INITIALIZER_LIST_
            TMatrixN<T, 4>(std::initializer_list<T>& list)
            {
                int count = 0;
                for (auto i = list.begin(); i != list.end(); i++)
                {
                    data[count / 4][count % 4] = *i;
                    count++;
                    if (count == 16)
                        break;
                }
            }
#endif

            TMatrixN<T, 4>(RowType &row0, RowType &row1, RowType &row2, RowType &row3)
            {
                data[0] = row0; data[1] = row1; data[2] = row2; data[3] = row3;
            }

            //  explicit TMatrixN<T, 4>( value_type row[][ 4 ] )
            //  {
            //    data[ 0 ].init( row[ 0 ] );
            //    data[ 1 ].init( row[ 1 ] );
            //    data[ 2 ].init( row[ 2 ] );
            //    data[ 3 ].init( row[ 3 ] );
            //  }

            explicit TMatrixN<T, 4>(value_type raw_data[16])
            {
                data[0].init(raw_data);
                data[1].init(raw_data + 4);
                data[2].init(raw_data + 8);
                data[3].init(raw_data + 12);
            }

            RowType& operator [] (int index) { assert(index < 4 && "TMatrixN : index < N -- Failed !");    return data[index]; }
            const RowType operator [] (int index) const { assert(index < 4 && "TMatrixN : index < N -- Failed !"); return data[index]; }

            operator const T* () const
            {
                return data[0];
            }

            friend KFORCE_INLINE TMatrixN<T, 4> operator * (const TMatrixN<T, 4> & m1, const TMatrixN<T, 4> & m2)
            {
                typename TMatrixN<T, 4>::RowType const SrcA0 = m1[0];
                typename TMatrixN<T, 4>::RowType const SrcA1 = m1[1];
                typename TMatrixN<T, 4>::RowType const SrcA2 = m1[2];
                typename TMatrixN<T, 4>::RowType const SrcA3 = m1[3];

                typename TMatrixN<T, 4>::RowType const SrcB0 = m2[0];
                typename TMatrixN<T, 4>::RowType const SrcB1 = m2[1];
                typename TMatrixN<T, 4>::RowType const SrcB2 = m2[2];
                typename TMatrixN<T, 4>::RowType const SrcB3 = m2[3];

                TMatrixN<T, 4> Result;
                Result[0] = SrcA0 * SrcB0[0] + SrcA1 * SrcB0[1] + SrcA2 * SrcB0[2] + SrcA3 * SrcB0[3];
                Result[1] = SrcA0 * SrcB1[0] + SrcA1 * SrcB1[1] + SrcA2 * SrcB1[2] + SrcA3 * SrcB1[3];
                Result[2] = SrcA0 * SrcB2[0] + SrcA1 * SrcB2[1] + SrcA2 * SrcB2[2] + SrcA3 * SrcB2[3];
                Result[3] = SrcA0 * SrcB3[0] + SrcA1 * SrcB3[1] + SrcA2 * SrcB3[2] + SrcA3 * SrcB3[3];
                return Result;
            }

            KFORCE_INLINE TMatrixN<T, 4> Inverse()
            {
                TMatrixN<T, 4> result;
                T temp[12];
                temp[0] = data[2][2] * data[3][3]; temp[1] = data[2][3] * data[3][2]; temp[2] = data[2][1] * data[3][3];
                temp[3] = data[2][3] * data[3][1]; temp[4] = data[2][1] * data[3][2]; temp[5] = data[2][2] * data[3][1];
                temp[6] = data[2][0] * data[3][3]; temp[7] = data[2][3] * data[3][0]; temp[8] = data[2][0] * data[3][2];
                temp[9] = data[2][2] * data[3][0]; temp[10] = data[2][0] * data[3][1]; temp[11] = data[2][1] * data[3][0];
                result[0][0] = temp[0] * data[1][1] + temp[3] * data[1][2] + temp[4] * data[1][3] - (temp[1] * data[1][1] + temp[2] * data[1][2] + temp[5] * data[1][3]);
                result[1][0] = temp[1] * data[1][0] + temp[6] * data[1][2] + temp[9] * data[1][3] - (temp[0] * data[1][0] + temp[7] * data[1][2] + temp[8] * data[1][3]);
                result[2][0] = temp[2] * data[1][0] + temp[7] * data[1][1] + temp[10] * data[1][3] - (temp[3] * data[1][0] + temp[6] * data[1][1] + temp[11] * data[1][3]);
                result[3][0] = temp[5] * data[1][0] + temp[8] * data[1][1] + temp[11] * data[1][2] - (temp[4] * data[1][0] + temp[9] * data[1][1] + temp[10] * data[1][2]);
                result[0][1] = temp[1] * data[0][1] + temp[2] * data[0][2] + temp[5] * data[0][3] - (temp[0] * data[0][1] + temp[3] * data[0][2] + temp[4] * data[0][3]);
                result[1][1] = temp[0] * data[0][0] + temp[7] * data[0][2] + temp[8] * data[0][3] - (temp[1] * data[0][0] + temp[6] * data[0][2] + temp[9] * data[0][3]);
                result[2][1] = temp[3] * data[0][0] + temp[6] * data[0][1] + temp[11] * data[0][3] - (temp[2] * data[0][0] + temp[7] * data[0][1] + temp[10] * data[0][3]);
                result[3][1] = temp[4] * data[0][0] + temp[9] * data[0][1] + temp[10] * data[0][2] - (temp[5] * data[0][0] + temp[8] * data[0][1] + temp[11] * data[0][2]);
                temp[0] = data[0][2] * data[1][3]; temp[1] = data[0][3] * data[1][2]; temp[2] = data[0][1] * data[1][3]; temp[3] = data[0][3] * data[1][1];
                temp[4] = data[0][1] * data[1][2]; temp[5] = data[0][2] * data[1][1]; temp[6] = data[0][0] * data[1][3]; temp[7] = data[0][3] * data[1][0];
                temp[8] = data[0][0] * data[1][2]; temp[9] = data[0][2] * data[1][0]; temp[10] = data[0][0] * data[1][1]; temp[11] = data[0][1] * data[1][0];
                result[0][2] = temp[0] * data[3][1] + temp[3] * data[3][2] + temp[4] * data[3][3] - (temp[1] * data[3][1] + temp[2] * data[3][2] + temp[5] * data[3][3]);
                result[1][2] = temp[1] * data[3][0] + temp[6] * data[3][2] + temp[9] * data[3][3] - (temp[0] * data[3][0] + temp[7] * data[3][2] + temp[8] * data[3][3]);
                result[2][2] = temp[2] * data[3][0] + temp[7] * data[3][1] + temp[10] * data[3][3] - (temp[3] * data[3][0] + temp[6] * data[3][1] + temp[11] * data[3][3]);
                result[3][2] = temp[5] * data[3][0] + temp[8] * data[3][1] + temp[11] * data[3][2] - (temp[4] * data[3][0] + temp[9] * data[3][1] + temp[10] * data[3][2]);
                result[0][3] = temp[2] * data[2][2] + temp[5] * data[2][3] + temp[1] * data[2][1] - (temp[4] * data[2][3] + temp[0] * data[2][1] + temp[3] * data[2][2]);
                result[1][3] = temp[8] * data[2][3] + temp[0] * data[2][0] + temp[7] * data[2][2] - (temp[6] * data[2][2] + temp[9] * data[2][3] + temp[1] * data[2][0]);
                result[2][3] = temp[6] * data[2][1] + temp[11] * data[2][3] + temp[3] * data[2][0] - (temp[10] * data[2][3] + temp[2] * data[2][0] + temp[7] * data[2][1]);
                result[3][3] = temp[10] * data[2][2] + temp[4] * data[2][0] + temp[9] * data[2][1] - (temp[8] * data[2][1] + temp[11] * data[2][2] + temp[5] * data[2][0]);

                T idet = T(1.0) / (data[0][0] * result[0][0] + data[0][1] * result[1][0] + data[0][2] * result[2][0] + data[0][3] * result[3][0]);

                result[0][0] *= idet; result[0][1] *= idet; result[0][2] *= idet; result[0][3] *= idet;
                result[1][0] *= idet; result[1][1] *= idet; result[1][2] *= idet; result[1][3] *= idet;
                result[2][0] *= idet; result[2][1] *= idet; result[2][2] *= idet; result[2][3] *= idet;
                result[3][0] *= idet; result[3][1] *= idet; result[3][2] *= idet; result[3][3] *= idet;

                return result;
            }

        protected:
            RowType data[4];
        };

        //---------------------------------------------------------
        template <typename T>
        KFORCE_INLINE T ToRadian(T const & degree)
        {
            T const pi = T(3.1415926535897932384626433832795);
            return degree * (pi / T(180));
        }

        template <typename T>
        KFORCE_INLINE T ToDegree(T const & rad)
        {
            const T pi = T(3.1415926535897932384626433832795);
            return rad * (T(180) / pi);
        }
        //-----------------------------------------------------------

        template <typename T>
        static KFORCE_INLINE TVector<T, 3> CrossProduct(const TVector<T, 3>& a, const TVector<T, 3>& b)
        {
            return TVector<T, 3>(a[1] * b[2] - b[1] * a[2],
                a[2] * b[0] - b[2] * a[0],
                a[0] * b[1] - b[0] * a[1]);
        }

        template <typename T>
        KFORCE_INLINE TMatrixN<T, 4> MakeIdentityMatrix()
        {
            static TMatrixN<T, 4> one;
            one[0][0] = T(1.0);
            one[1][1] = T(1.0);
            one[2][2] = T(1.0);
            one[3][3] = T(1.0);
            return one;
        }

        template <typename T>
        KFORCE_INLINE TMatrixN<T, 4> Rotate(const TVector<T, 3> &vec, T angle, const TMatrixN<T, 4>& m)
        {
            T rad = ToRadian(angle);
            T c = cos(rad);
            T s = sin(rad);

            TVector<T, 3> axis = Normalize(vec);
            TVector<T, 3> temp = axis * (T(1) - c);

            TMatrixN<T, 4> Rotate;
            Rotate[0][0] = c + temp[0] * axis[0];
            Rotate[0][1] = 0 + temp[0] * axis[1] + s * axis[2];
            Rotate[0][2] = 0 + temp[0] * axis[2] - s * axis[1];

            Rotate[1][0] = 0 + temp[1] * axis[0] - s * axis[2];
            Rotate[1][1] = c + temp[1] * axis[1];
            Rotate[1][2] = 0 + temp[1] * axis[2] + s * axis[0];

            Rotate[2][0] = 0 + temp[2] * axis[0] + s * axis[1];
            Rotate[2][1] = 0 + temp[2] * axis[1] - s * axis[0];
            Rotate[2][2] = c + temp[2] * axis[2];

            TMatrixN<T, 4> Result;
            Result[0] = m[0] * Rotate[0][0] + m[1] * Rotate[0][1] + m[2] * Rotate[0][2];
            Result[1] = m[0] * Rotate[1][0] + m[1] * Rotate[1][1] + m[2] * Rotate[1][2];
            Result[2] = m[0] * Rotate[2][0] + m[1] * Rotate[2][1] + m[2] * Rotate[2][2];
            Result[3] = m[3];
            return Result;
        }

        /// Make A Rotation Matrix
        /// \brief MakeRotationMatrix
        /// \param vec
        /// \param angle
        /// \return Origin Matrix Multiplied Rotation Matrix
        template <typename T>
        KFORCE_INLINE TMatrixN<T, 4> MakeRotationMatrix(const TVector<T, 3> &vec, T angle)
        {
            return Rotate(vec, angle, MakeIdentityMatrix<T>());
        }

        /// Translate The Matrix "m" by Vector v
        /// \brief Translate
        /// \param v
        /// \param m
        /// \return Translated Matrix
        template <typename T>
        KFORCE_INLINE TMatrixN<T, 4> Translate(const TVector<T, 3> &v, const TMatrixN<T, 4>& m)
        {
            TMatrixN<T, 4> res(m);
            res[3] = m[0] * v[0] + m[1] * v[1] + m[2] * v[2] + m[3];
            return res;
        }

        ///
        /// \brief MakeTranslationMatrix
        /// \param v Translation Vector 3
        /// \return translated Matrix
        template <typename T>
        KFORCE_INLINE TMatrixN<T, 4> MakeTranslationMatrix(T x, T y, T z)
        {
            TMatrixN<T, 4> mat4;
            mat4[0] = { T(1.0), T(0.0), T(0.0), T(0.0) };
            mat4[1] = { T(0.0), T(1.0), T(0.0), T(0.0) };
            mat4[2] = { T(0.0), T(0.0), T(1.0), T(0.0) };
            mat4[3] = { x, y, z, T(1.0) };

            return mat4;
        }

        ///
        /// \brief MakeTranslationMatrix
        /// \param v Translation Vector 3
        /// \return translated Matrix
        template <typename T>
        KFORCE_INLINE TMatrixN<T, 4> MakeTranslationMatrix(TVector<T, 3> & vec)
        {
            return MakeTranslationMatrix(vec[0], vec[1], vec[2]);
        }

        //! \fn template <typename T> KFORCE_INLINE TMatrixN<T, 4> MakeFrustum( T left, T right, T bottom, T top, T n, T f )
        //! \brief  Makes a frustum matrix For Light...
        //! \tparam T can be float, double.
        //! \param  left    The left plane distance.
        //! \param  right   The right plane distance.
        //! \param  bottom  The bottom plane distance.
        //! \param  top     The top plane distance.
        //! \param  n       near plane distance
        //! \param  f       far plane distance
        //!
        //! \return A Frustum TMatrixN&lt;T,4&gt;
        template <typename T>
        KFORCE_INLINE TMatrixN<T, 4> MakeFrustum(
            T left, T right,
            T bottom, T top,
            T n, T f)
        {
            TMatrixN<T, 4> result(MakeIdentityMatrix<T>());

            if ((right == left) ||
                (top == bottom) ||
                (n == f) ||
                (n < T(0.0)) ||
                (f < T(0.0)))
                return result;

            result[0][0] = (T(2.0) * n) / (right - left);
            result[1][1] = (T(2.0) * n) / (top - bottom);

            result[2][0] = (right + left) / (right - left);
            result[2][1] = (top + bottom) / (top - bottom);
            result[2][2] = -(f + n) / (f - n);
            result[2][3] = -T(1.0);

            result[3][2] = -(T(2.0) * f * n) / (f - n);
            result[3][3] = T(0.0);

            return result;
        }

        /// template function Perspective
        /// \brief Perspective
        /// \param fov in Y
        /// \param ratio
        /// \param near_plane
        /// \param far_plane
        /// \return Matrix4X4 Perspective Matrix
        template <typename T, class MatType = TMatrixN<T, 4> >
        KFORCE_INLINE MatType Perspective(T fovY, T ratio, T near_plane, T far_plane)
        {
            MatType result;
            T q = T(1.0) / T(::tan(ToRadian(T(0.5) * fovY)));
            T A = q / ratio;
            T B = (near_plane + far_plane) / (near_plane - far_plane);
            T C = (T(2.0) * near_plane * far_plane) / (near_plane - far_plane);
            result[0] = { A, T(0.0), T(0.0), T(0.0) };
            result[1] = { T(0.0), q, T(0.0), T(0.0) };
            result[2] = { T(0.0), T(0.0), B, T(-1.0) };
            result[3] = { T(0.0), T(0.0), C, T(0.0) };
            return result;
        }

        //! \fn template < typename T > KFORCE_INLINE TMatrixN<T, 4> LookAt( const TVector<T, 3> & eye, const TVector<T, 3> & center, const TVector<T, 3> & up )
        //! \brief  LookAt for Camera
        //! \tparam T Generic type parameter.
        //! \param  eye     The eye.
        //! \param  center  The center.
        //! \param  up      The up.
        //!
        //! \return A TMatrixN&lt;T,4&gt;
        template < typename T >
        KFORCE_INLINE TMatrixN<T, 4> LookAt(
            const TVector<T, 3> & eye,
            const TVector<T, 3> & center,
            const TVector<T, 3> & up)
        {
            TVector<T, 3> f = Normalize(center - eye);
            TVector<T, 3> u = Normalize(up);
            TVector<T, 3> s = Normalize(CrossProduct(f, u));
            u = CrossProduct(s, f);

            TMatrixN<T, 4> Result(MakeIdentityMatrix<T>());
            Result[0][0] = s[0];
            Result[1][0] = s[1];
            Result[2][0] = s[2];
            Result[0][1] = u[0];
            Result[1][1] = u[1];
            Result[2][1] = u[2];
            Result[0][2] = -f[0];
            Result[1][2] = -f[1];
            Result[2][2] = -f[2];
            Result[3][0] = -DotProduct(s, eye);
            Result[3][1] = -DotProduct(u, eye);
            Result[3][2] = DotProduct(f, eye);
            return Result;
        }


        template <>
        class TMatrixN<float, 4>
        {
        public:
            typedef float value_type;
            enum { N = 4 };
            typedef TVector<float, 4> RowType;

            TMatrixN() {}

            KFORCE_INLINE TMatrixN(RowType &row0, RowType &row1, RowType &row2, RowType &row3)
            {
                data[0] = row0; data[1] = row1; data[2] = row2; data[3] = row3;
            }
            /*
            KFORCE_INLINE TMatrixN(std::initializer_list<float>& list)
            {
                int count = 0;
                for (auto i = list.begin(); i != list.end(); i++)
                {
                    data[count / 4][count % 4] = *i;
                    count++;
                    if (count == 16)
                        break;
                }
            }*/

            KFORCE_INLINE explicit TMatrixN(value_type raw_data[16])
            {
                data[0].init(raw_data);
                data[1].init(raw_data + 4);
                data[2].init(raw_data + 8);
                data[3].init(raw_data + 12);
            }

            KFORCE_INLINE RowType& operator [] (int index) { assert(index < 4 && "TMatrixN : index < _N -- Failed !");    return data[index]; }
            KFORCE_INLINE const RowType operator [] (int index) const { assert(index < 4 && "TMatrixN : index < _N -- Failed !"); return data[index]; }

            KFORCE_INLINE operator const float* () const
            {
                return data[0];
            }

        private:
            TVector<float, 4> data[4];
        };

        KFORCE_INLINE TMatrixN<float, 4> operator * (const TMatrixN<float, 4> &a, const TMatrixN<float, 4> &b)
        {
            TMatrixN<float, 4> res;
            simd::MatrixMultiply(&res, (void*)&a, (void*)&b);
            return res;
        }

        KFORCE_INLINE TVector<float, 4> operator * (const TMatrixN<float, 4> &a, const TVector<float, 4> &b)
        {
            TVector<float, 4> res;
            simd::MatrixVectorMultiply(&res, (void*)&a, (void*)&b);
            return res;
        }

    } // math
}


#endif
