#pragma once

#ifndef __k3d_Vector_h__
#define __k3d_Vector_h__

#include "CoreMinimal.h"

#include <cmath>

#if K3DCOMPILER_MSVC
#pragma warning(disable:4201) // disable anonymous union warnings
#endif

namespace k3d
{
    namespace math
    {
        template <typename T, int N>
        struct TVector;

        template <typename T, int N>
        struct TVector
        {
            TVector() 
            { 
                MemoryFill(m_data, 0, N * sizeof(T));
            }
            template <class U>
            TVector(const U *ptr) { this->template init<U>(ptr); }

            template <class U>
            void init(U *s_offset)
            {
                assert(sizeof(U) <= sizeof(T) && s_offset);
                MemoryCopy(m_data, s_offset, N * sizeof(U));
            }
            template <class U>
            void init(U *s_offset, int n)
            {
                assert(s_offset && n <= N && n * sizeof(U) <= N * sizeof(T));
                MemoryCopy(m_data, s_offset, n * sizeof(U));
            }

            enum { Len = N };
            typedef T value_type;
            KFORCE_INLINE T& operator [] (int index) { assert(index < N && "TVector : index < N -- Failed !"); return m_data[index]; }
            KFORCE_INLINE const T operator [] (int index)const { assert(index < N && "TVector : index < N -- Failed !"); return m_data[index]; }

            operator const T* ()
            {
                return &m_data[0];
            }

        protected:
            T m_data[N];
        };

        template <typename T, int N>
        KFORCE_INLINE const T Length(const TVector<T, N>& rhs) {
            return T(sqrt(1.0*DotProduct(rhs, rhs)));
        }

        template <typename T, int N>
        TVector<T, N> Maximize(TVector<T, N> const &v1, TVector<T, N> const &v2)
        {
            TVector<T, N> result;
            for (int i = 0; i<N; i++)
                result[i] = (v1[i]>v2[i]) ? v1[i] : v2[i];
            return result;
        }

        template <typename T, int N>
        TVector<T, N> Minimize(TVector<T, N> const &v1, TVector<T, N> const &v2)
        {
            TVector<T, N> result;
            for (int i = 0; i<N; i++)
                result[i] = (v1[i]<v2[i]) ? v1[i] : v2[i];
            return result;
        }

        ///////////////////////////Operator Implement//////////////////////////////
        template <typename T, int N>
        KFORCE_INLINE TVector<T, N> operator + (TVector<T, N> const & a, TVector<T, N> const & b)
        {
            TVector<T, N> result;
            for (int i = 0; i < N; i++) result[i] = a[i] + b[i];
            return result;
        }
        template <typename T, int N>
        KFORCE_INLINE TVector<T, N> operator - (TVector<T, N> const & a, TVector<T, N> const & b)
        {
            TVector<T, N> result;
            for (int i = 0; i < N; i++) result[i] = a[i] - b[i];
            return result;
        }

        template <typename T, int N>
        KFORCE_INLINE TVector<T, N> operator + (TVector<T, N> const & a, T const & b)
        {
            TVector<T, N> result;
            for (int i = 0; i < N; i++) result[i] = a[i] + b;
            return result;
        }

        template <typename T, int N>
        KFORCE_INLINE TVector<T, N> operator - (TVector<T, N> const & a, T const & b)
        {
            TVector<T, N> result;
            for (int i = 0; i < N; i++) result[i] = a[i] - b;
            return result;
        }

        template <typename T, int N>
        KFORCE_INLINE TVector<T, N> operator * (TVector<T, N> const & a, TVector<T, N> const & b)
        {
            TVector<T, N> result;
            for (int i = 0; i < N; i++) result[i] = a[i] * b[i];
            return result;
        }

        template <typename T, int N>
        KFORCE_INLINE TVector<T, N> operator * (TVector<T, N> const & a, typename TVector<T, N>::value_type const & b)
        {
            TVector<T, N> result;
            for (int i = 0; i < N; i++)
                result[i] = a[i] * b;
            return result;
        }

        template <typename T, int N>
        KFORCE_INLINE TVector<T, N> operator / (TVector<T, N> const & a, typename TVector<T, N>::value_type const & b)
        {
            TVector<T, N> result;
            for (int i = 0; i < N; i++)
                result[i] = a[i] / b;
            return result;
        }

        // [0] DotProduct------------------------------------------------------------------------------
        template <int N, class T>
        class tDotProduct {
        public:
            static typename T::value_type result(T a, T b) {
                return a[N - 1] * b[N - 1] + tDotProduct<N - 1, T>::result(a, b);
            }
        };
        template <class T>
        class tDotProduct<1, T> {
        public:
            static typename T::value_type result(T a, T b) {
                return a[0] * b[0];
            }
        };
        template <class T>
        KFORCE_INLINE typename T::value_type DotProduct(T a, T b)
        {
            return tDotProduct<T::Len, T>::result(a, b);
        }


        template <typename T>
        struct TVector<T, 2>
        {
            enum { Len = 2 };
            typedef T value_type;
            TVector() { m_data[0] = 0; m_data[1] = 0; }
            TVector(T x, T y) { m_data[0] = x; m_data[1] = y; }
            TVector(const T *ptr) { this->template init<T>(ptr); }

            T& operator [] (int index) { assert(index < 2 && "tVector2 : index < 2 -- Failed !"); return m_data[index]; }
            const T operator [] (int index)const { assert(index < 2 && "tVector2 : index < N -- Failed !"); return m_data[index]; }

            const T X() const { return m_data[0]; }
            const T Y() const { return m_data[1]; }
            //-------------------------------------------------------
            template <class U>
            void init(U *s_offset)
            {
                assert(sizeof(U) <= sizeof(T) && s_offset);
                MemoryCopy(m_data, s_offset, 2 * sizeof(U));
            }
            //-------------------------------------------------------
            union {
                T m_data[2];
                struct {
                    T x, y;
                };
            };
        };

        template <typename T>
        struct TVector<T, 3>
        {
            enum { Len = 3 };
            typedef T value_type;

            TVector() { m_data[0] = 0; m_data[1] = 0; m_data[2] = 0; }
            TVector(T one) { m_data[0] = one; m_data[1] = one; m_data[2] = one; }
            TVector(T x, T y, T z) { m_data[0] = x; m_data[1] = y;  m_data[2] = z; }
            TVector(const T *ptr) { this->template init<T>(ptr); }

            KFORCE_INLINE T& operator [] (int index) { assert(index < 3 && "tVector3 : index < 3 -- Failed !"); return m_data[index]; }
            KFORCE_INLINE const T operator [] (int index)const { assert(index < 3 && "tVector3 : index < 3 -- Failed !"); return m_data[index]; }

            operator const T* ()
            {
                return &m_data[0];
            }

            //-------------------------------------------------------
            template <class U>
            void init(U *s_offset)
            {
                assert(sizeof(U) <= sizeof(T) && s_offset);
                MemoryCopy(m_data, s_offset, 4 * sizeof(U));
            }
            //-------------------------------------------------------

            KFORCE_INLINE TVector & operator += (const TVector & other)
            {
                this->x += other.x; this->y += other.y; this->z += other.z;
                return *this;
            }

            KFORCE_INLINE TVector & operator -= (const TVector & other)
            {
                this->x -= other.x; this->y -= other.y; this->z -= other.z;
                return *this;
            }

            KFORCE_INLINE TVector & operator -= (const T & other)
            {
                this->x -= other; this->y -= other; this->z -= other;
                return *this;
            }

            // No help for anonymous union but For reading
            union {
                T m_data[3];
                struct {
                    T x, y, z;
                };
            };
        };

        template <typename T>
        TVector<T, 3> Normalize(const TVector<T, 3> &vec)
        {
            /*T len_inv = T( 1.0 ) / T(vec.Length());
            return TVector<T, 3>( vec[ 0 ] * len_inv, vec[ 1 ] * len_inv, vec[ 2 ] * len_inv );
            */
            typename TVector<T, 3>::value_type sqr = vec.x * vec.x + vec.y * vec.y + vec.z * vec.z;
            return vec * T(1.0) / sqrt(sqr);;
        }

        template <typename T>
        struct TVector<T, 4>
        {
            enum { Len = 4 };
            typedef T value_type;
            TVector() { m_data[0] = 0; m_data[1] = 0; m_data[2] = 0; m_data[3] = 0; }
            TVector(T x, T y, T z, T w) { m_data[0] = x; m_data[1] = y;  m_data[2] = z; m_data[3] = w; }
            TVector(const TVector<T, 3> &vec, T w) { m_data[0] = vec[0]; m_data[1] = vec[1]; m_data[2] = vec[2]; m_data[0] = w; }
            TVector(const T *ptr) { this->template init<T>(ptr); }

            T& operator [] (int index) { assert(index < 4 && "tVector4 : index < 4 -- Failed !"); return m_data[index]; }
            const T operator [] (int index)const { assert(index < 4 && "tVector4 : index < 4 -- Failed !"); return m_data[index]; }

            operator const T* () const
            {
                return &m_data[0];
            }

            operator TVector<T, 3>() const
            {
                return TVector<T, 3>(x, y, z);
            }

            KFORCE_INLINE TVector<T, 3> ToVec3() const
            {
                return TVector<T, 3>(this->x, this->y, this->z);
            }

            //-------------------------------------------------------
            template <class U>
            KFORCE_INLINE void init(const U *s_offset)
            {
                assert(sizeof(U) <= sizeof(T) && s_offset);
                MemoryCopy(m_data, s_offset, 4 * sizeof(U));
            }

            //-------------------------------------------------------
        protected: // No help for anonymous union but For reading
            union {
                T m_data[4];
                struct {
                    T x, y, z, w;
                };
            };
        }; // TVector<4>

        // ---------------- SIMD -------------------//
        template <>
        class TVector<float, 4>
        {
        public:
            enum { Len = 4 };
            typedef float value_type;

            KFORCE_INLINE TVector() { data = simd::MakeFloat4(0.f); }
            KFORCE_INLINE TVector(V4F _simd) { data = _simd; }
            KFORCE_INLINE TVector(float x, float y, float z, float w) { data = simd::MakeFloat4(x, y, z, w); }

            KFORCE_INLINE explicit TVector(const float *ptr) { this->init(ptr); }
            KFORCE_INLINE explicit TVector(float ones) { data = simd::MakeFloat4(ones); }
            KFORCE_INLINE TVector(const TVector<float, 3>& vec3, float w)
            {
                data = simd::MakeFloat4(vec3[0], vec3[2], vec3[2], w);
            }

            KFORCE_INLINE void init(const float *ptr) { data = simd::LoadFloat4(ptr); }

            KFORCE_INLINE float& operator [] (int index) { assert(index < 4 && "tVector4 : index < 4 -- Failed !"); return d[index]; }
            KFORCE_INLINE const float operator [] (int index) const { assert(index < 4 && "tVector4 : index < 4 -- Failed !"); return d[index]; }

            operator const float* () const
            {
                return &d[0];
            }

            KFORCE_INLINE operator TVector<float, 3>() const
            {
                return TVector<float, 3>(d[0], d[1], d[2]);
            }

            KFORCE_INLINE TVector<float, 3> ToVec3()
            {
                return TVector<float, 3>(d[0], d[1], d[2]);
            }

            KFORCE_INLINE operator V4F () const
            {
                return data;
            }

        private:
            union {
                float d[4];
                V4F data;
            };
        };

        template <>
        KFORCE_INLINE TVector<float, 4> Minimize(TVector<float, 4> const &v1, TVector<float, 4> const &v2)
        {
            return simd::Min(v1, v2);
        }

        template <>
        KFORCE_INLINE TVector<float, 4> Maximize(TVector<float, 4> const &v1, TVector<float, 4> const &v2)
        {
            return simd::Max(v1, v2);
        }
        /*
        KFORCE_INLINE float DotProduct(const TVector<float, 4>& a, const TVector<float, 4>& b)
        {
            V4F result = Dot((float4)a, (float4)b);
#if __ENABLE_SSE__
            return result.m128_f32[0];
#endif
        }
        */

        KFORCE_INLINE TVector<float, 4> operator / (const TVector<float, 4> &a, const float factor)
        {
            float f = 1.0f / factor;
            V4F res = simd::Multiply((V4F)a, simd::MakeFloat4(f));
            return TVector<float, 4>(res);
        }
    }
}

#endif
