#pragma once
#ifndef __k3d_Simd_h__
#define __k3d_Simd_h__
#include "Types.h"
#if K3D_USE_SSE
#define _MM_PERM2_X			0
#define _MM_PERM2_Y			1
#define _MM_PERM2_Z			2
#define _MM_PERM2_W			3
#define _MM_PERM2(X,Y,Z,W)	_MM_SHUFFLE(_MM_PERM2_ ## W,_MM_PERM2_ ## Z,_MM_PERM2_ ## Y,_MM_PERM2_ ## X)
#define _MM_SWIZZLE(V,X,Y,Z,W) _mm_shuffle_ps(V,V,_MM_PERM2(X,Y,Z,W))
#endif

namespace k3d
{
    namespace simd
    {
        KFORCE_INLINE V4F MakeFloat4(float v) {
#if K3D_USE_SSE
            return _mm_set_ps1(v);
#elif K3D_USE_NEON
            return vdupq_n_f32(v);
#else
#endif
        }

        KFORCE_INLINE V4F MakeFloat4(float x, float y, float z, float w) {
#if K3D_USE_SSE
            return _mm_setr_ps(x, y, z, w);
#elif K3D_USE_NEON
            union { V4F v; float f[4]; } shadow_float4;
            shadow_float4.f[0] = x;
            shadow_float4.f[1] = y;
            shadow_float4.f[2] = z;
            shadow_float4.f[3] = w;
            return shadow_float4.v;
#else
#endif
        }

        KFORCE_INLINE V4F LoadFloat4(const float* Ptr)
        {
#if K3D_USE_SSE
            return _mm_load_ps(Ptr);
#elif K3D_USE_NEON
            return vld1q_f32(Ptr);
#else
#endif
        }

        KFORCE_INLINE float GetFloat(V4F & a, int c)
        {
            return *((float*)(&a) + c);
        }

        KFORCE_INLINE I32 GetInt(V4I & a, int c)
        {
            return *((I32*)(&a) + c);
        }

        /// Logic Ops

        KFORCE_INLINE V4I And(V4I & a, V4I & b)
        {
#if K3D_USE_SSE
            return _mm_and_si128(a, b);
#elif K3D_USE_NEON
            return vandq_s32(a, b);
#else
#endif
        }

        KFORCE_INLINE V4F And(V4F & a, V4F & b)
        {
#if K3D_USE_SSE
            return _mm_and_ps(a, b);
#elif K3D_USE_NEON
            return (V4F)vandq_u32((V4I)a, (V4I)b);
#else
#endif
        }

        KFORCE_INLINE V4I Or(V4I & a, V4I & b)
        {
#if K3D_USE_SSE
            return _mm_or_si128(a, b);
#elif K3D_USE_NEON
            return vorrq_s32(a, b);
#else
#endif
        }

        KFORCE_INLINE V4F Or(V4F & a, V4F & b)
        {
#if K3D_USE_SSE
            return _mm_or_ps(a, b);
#elif K3D_USE_NEON
            return (V4F)vorrq_u32((V4I)a, (V4I)b);
#else
#endif
        }

        KFORCE_INLINE V4I Xor(V4I & a, V4I & b)
        {
#if K3D_USE_SSE
            return _mm_xor_si128(a, b);
#elif K3D_USE_NEON
            return veorq_s32(a, b);
#else
#endif
        }

        KFORCE_INLINE V4F Xor(V4F & a, V4F & b)
        {
#if K3D_USE_SSE
            return _mm_xor_ps(a, b);
#elif K3D_USE_NEON
            return (V4F)veorq_u32((V4I)a, (V4I)b);
#else
#endif
        }
        /*
        KFORCE_INLINE V4I Not(V4I & a)
        {
#if K3D_USE_SSE
#elif K3D_USE_NEON
            return vmvnq_s32(a);
#else
#endif
        }

        KFORCE_INLINE V4I AndNot(V4I & a, V4I & b)
        {
#if K3D_USE_SSE
#elif K3D_USE_NEON
            return vandq_s32(vmvnq_s32(a), b);
#else
#endif
        }
        */
        /// Logic Ops End

        /// Comparison


        /// Comparison End

        KFORCE_INLINE V4F Select(V4F & a, V4F & b, V4F & m)
        {
#if K3D_USE_SSE
            return _mm_xor_ps(b, _mm_and_ps(m, _mm_xor_ps(a, b)));
#elif K3D_USE_NEON
            return vbslq_f32((V4I)m, a, b);
#else
#endif
        }

        KFORCE_INLINE V4I Select(V4I & a, V4I & b, V4I & m)
        {
#if K3D_USE_SSE
            return _mm_xor_si128(b, _mm_and_si128(m, _mm_xor_si128(a, b)));
#elif K3D_USE_NEON
            //return vloadq_f32(Ptr);
            return V4I();
#else
#endif
        }

        KFORCE_INLINE V4F Add(V4F & a, V4F & b) {
#if K3D_USE_SSE
            return _mm_add_ps(a, b);
#elif K3D_USE_NEON
            return vaddq_f32(a, b);
#else

#endif
        }

        KFORCE_INLINE V4I Add(V4I & a, V4I & b) {
#if K3D_USE_SSE
            return _mm_add_epi32(a, b);
#elif K3D_USE_NEON
            return vaddq_s32(a, b);
#else

#endif
        }

        KFORCE_INLINE V4F Subtract(V4F & a, V4F & b) {
#if K3D_USE_SSE
            return _mm_sub_ps(a, b);
#elif K3D_USE_NEON
            return vsubq_s32(a, b);
#else

#endif
        }

        KFORCE_INLINE V4I Subtract(V4I & a, V4I & b) {
#if K3D_USE_SSE
            return _mm_sub_epi32(a, b);
#elif K3D_USE_NEON
            return vsubq_f32(a, b);
#else

#endif
        }

        KFORCE_INLINE V4F Multiply(V4F const & a, V4F const & b) {
#if K3D_USE_SSE
            return _mm_mul_ps(a, b);
#elif K3D_USE_NEON
            return vmulq_f32(a, b);
#else
#endif
        }
        /* using define instead
        // unsigned
        KFORCE_INLINE V4I ShiftLeft(V4I const& a, const int count)
        {
#if K3D_USE_SSE
            return _mm_slli_epi32(a, count);
#elif K3D_USE_NEON
            return vshlq_n_u32(a, count);
#else
            V4I res;
            res.u32[0] = a.u32[0] << count;
            res.u32[1] = a.u32[1] << count;
            res.u32[2] = a.u32[2] << count;
            res.u32[3] = a.u32[3] << count;
            return res;
#endif
        }

        // unsigned
        KFORCE_INLINE V4I ShiftRight(V4I const& a, const int count)
        {
#if K3D_USE_SSE
            return _mm_srli_epi32(a, count);
#elif K3D_USE_NEON
            return vshrq_n_u32(a, count);
#else
            V4I res;
            res.u32[0] = a.u32[0] >> count;
            res.u32[1] = a.u32[1] >> count;
            res.u32[2] = a.u32[2] >> count;
            res.u32[3] = a.u32[3] >> count;
            return res;
#endif
        }
        */
        KFORCE_INLINE V4F Min(V4F const & a, V4F const & b) {
#if K3D_USE_SSE
            return _mm_min_ps(a, b);
#elif K3D_USE_NEON
            return vminq_f32(a, b);
#else

#endif
        }

        KFORCE_INLINE V4F Max(V4F const & a, V4F const & b) {
#if K3D_USE_SSE
            return _mm_max_ps(a, b);
#elif K3D_USE_NEON
            return vmaxq_f32(a, b);
#else

#endif
        }

        KFORCE_INLINE V4F Dot(V4F const & a, V4F const & b) {
#if K3D_USE_SSE
            V4F dot = _mm_mul_ps(a, b);
            V4F tmp = _mm_shuffle_ps(dot, dot, _MM_SHUFFLE(0, 0, 3, 2));
            dot = _mm_add_ps(dot, tmp);
            tmp = _mm_shuffle_ps(dot, dot, _MM_SHUFFLE(0, 0, 0, 1));
            return _mm_add_ps(dot, tmp);
#elif K3D_USE_NEON
#else
#endif
        }

        KFORCE_INLINE V4F Reciprocal(V4F const & v)
        {
#if K3D_USE_SSE
            return _mm_rcp_ps(v);
#elif K3D_USE_NEON
            return vrecpeq_f32(v);
#else
#endif
        }

        KFORCE_INLINE V4F ReciprocalSqrt(V4F const & v)
        {
#if K3D_USE_SSE
            return _mm_rsqrt_ps(v);
#elif K3D_USE_NEON
            return vrsqrteq_f32(v);
#else
#endif
        }

        KFORCE_INLINE V4F ReciprocalLength(V4F const & v) {
            const V4F & recip = Dot(v, v);
            return ReciprocalSqrt(recip);
        }

        KFORCE_INLINE V4F Normalize(V4F const & v) {
            return Multiply(v, ReciprocalLength(v));
        }


#if K3D_USE_SSE
        KFORCE_INLINE V4F _mm_rcp_ss_nr(V4F v)
        {
            V4F iv = _mm_rcp_ss(v);
            return _mm_sub_ss(_mm_add_ss(iv, iv), _mm_mul_ss(v, _mm_mul_ss(iv, iv)));
        }
#endif

        // row major vector?
        KFORCE_INLINE void MatrixVectorMultiply(void* result, void* a, void* b) {
#if K3D_USE_SSE
#elif K3D_USE_NEON

#else // Dumy imp

#endif
        }

        // column major vector
        KFORCE_INLINE void VectorMatrixMultiply(void* result, void* a, void* b) {
#if K3D_USE_SSE
#elif K3D_USE_NEON

#else // Dumy imp

#endif
        }

        KFORCE_INLINE void MatrixMultiply(void* result, void* a, void* b) {
#if K3D_USE_SSE
            V4F *in1 = (V4F*)a;
            V4F *in2 = (V4F*)b;
            V4F *out = (V4F*)result;
            {
                V4F e0 = _mm_shuffle_ps(in2[0], in2[0], _MM_SHUFFLE(0, 0, 0, 0));
                V4F e1 = _mm_shuffle_ps(in2[0], in2[0], _MM_SHUFFLE(1, 1, 1, 1));
                V4F e2 = _mm_shuffle_ps(in2[0], in2[0], _MM_SHUFFLE(2, 2, 2, 2));
                V4F e3 = _mm_shuffle_ps(in2[0], in2[0], _MM_SHUFFLE(3, 3, 3, 3));

                V4F m0 = _mm_mul_ps(in1[0], e0);
                V4F m1 = _mm_mul_ps(in1[1], e1);
                V4F m2 = _mm_mul_ps(in1[2], e2);
                V4F m3 = _mm_mul_ps(in1[3], e3);

                V4F a0 = _mm_add_ps(m0, m1);
                V4F a1 = _mm_add_ps(m2, m3);
                V4F a2 = _mm_add_ps(a0, a1);

                out[0] = a2;
            }

            {
                V4F e0 = _mm_shuffle_ps(in2[1], in2[1], _MM_SHUFFLE(0, 0, 0, 0));
                V4F e1 = _mm_shuffle_ps(in2[1], in2[1], _MM_SHUFFLE(1, 1, 1, 1));
                V4F e2 = _mm_shuffle_ps(in2[1], in2[1], _MM_SHUFFLE(2, 2, 2, 2));
                V4F e3 = _mm_shuffle_ps(in2[1], in2[1], _MM_SHUFFLE(3, 3, 3, 3));

                V4F m0 = _mm_mul_ps(in1[0], e0);
                V4F m1 = _mm_mul_ps(in1[1], e1);
                V4F m2 = _mm_mul_ps(in1[2], e2);
                V4F m3 = _mm_mul_ps(in1[3], e3);

                V4F a0 = _mm_add_ps(m0, m1);
                V4F a1 = _mm_add_ps(m2, m3);
                V4F a2 = _mm_add_ps(a0, a1);

                out[1] = a2;
            }

            {
                V4F e0 = _mm_shuffle_ps(in2[2], in2[2], _MM_SHUFFLE(0, 0, 0, 0));
                V4F e1 = _mm_shuffle_ps(in2[2], in2[2], _MM_SHUFFLE(1, 1, 1, 1));
                V4F e2 = _mm_shuffle_ps(in2[2], in2[2], _MM_SHUFFLE(2, 2, 2, 2));
                V4F e3 = _mm_shuffle_ps(in2[2], in2[2], _MM_SHUFFLE(3, 3, 3, 3));

                V4F m0 = _mm_mul_ps(in1[0], e0);
                V4F m1 = _mm_mul_ps(in1[1], e1);
                V4F m2 = _mm_mul_ps(in1[2], e2);
                V4F m3 = _mm_mul_ps(in1[3], e3);

                V4F a0 = _mm_add_ps(m0, m1);
                V4F a1 = _mm_add_ps(m2, m3);
                V4F a2 = _mm_add_ps(a0, a1);

                out[2] = a2;
            }

            {
                V4F e0 = _mm_shuffle_ps(in2[3], in2[3], _MM_SHUFFLE(0, 0, 0, 0));
                V4F e1 = _mm_shuffle_ps(in2[3], in2[3], _MM_SHUFFLE(1, 1, 1, 1));
                V4F e2 = _mm_shuffle_ps(in2[3], in2[3], _MM_SHUFFLE(2, 2, 2, 2));
                V4F e3 = _mm_shuffle_ps(in2[3], in2[3], _MM_SHUFFLE(3, 3, 3, 3));

                V4F m0 = _mm_mul_ps(in1[0], e0);
                V4F m1 = _mm_mul_ps(in1[1], e1);
                V4F m2 = _mm_mul_ps(in1[2], e2);
                V4F m3 = _mm_mul_ps(in1[3], e3);

                V4F a0 = _mm_add_ps(m0, m1);
                V4F a1 = _mm_add_ps(m2, m3);
                V4F a2 = _mm_add_ps(a0, a1);

                out[3] = a2;
            }
#elif K3D_USE_NEON
            const V4F *A = (const V4F *)a;
            const V4F *B = (const V4F *)b;
            V4F *R = (V4F *)result;
            V4F tmp_lane, R0, R1, R2, R3;

            tmp_lane = vmulq_lane_f32(B[0], vget_low_f32(A[0]), 0);
            tmp_lane = vmlaq_lane_f32(tmp_lane, B[1], vget_low_f32(A[0]), 1);
            tmp_lane = vmlaq_lane_f32(tmp_lane, B[2], vget_high_f32(A[0]), 0);
            R0 = vmlaq_lane_f32(tmp_lane, B[3], vget_high_f32(A[0]), 1);

            tmp_lane = vmulq_lane_f32(B[0], vget_low_f32(A[1]), 0);
            tmp_lane = vmlaq_lane_f32(tmp_lane, B[1], vget_low_f32(A[1]), 1);
            tmp_lane = vmlaq_lane_f32(tmp_lane, B[2], vget_high_f32(A[1]), 0);
            R1 = vmlaq_lane_f32(tmp_lane, B[3], vget_high_f32(A[1]), 1);

            tmp_lane = vmulq_lane_f32(B[0], vget_low_f32(A[2]), 0);
            tmp_lane = vmlaq_lane_f32(tmp_lane, B[1], vget_low_f32(A[2]), 1);
            tmp_lane = vmlaq_lane_f32(tmp_lane, B[2], vget_high_f32(A[2]), 0);
            R2 = vmlaq_lane_f32(tmp_lane, B[3], vget_high_f32(A[2]), 1);

            tmp_lane = vmulq_lane_f32(B[0], vget_low_f32(A[3]), 0);
            tmp_lane = vmlaq_lane_f32(tmp_lane, B[1], vget_low_f32(A[3]), 1);
            tmp_lane = vmlaq_lane_f32(tmp_lane, B[2], vget_high_f32(A[3]), 0);
            R3 = vmlaq_lane_f32(tmp_lane, B[3], vget_high_f32(A[3]), 1);

            // Store result
            R[0] = R0;
            R[1] = R1;
            R[2] = R2;
            R[3] = R3;
#else
#endif
        }

        KFORCE_INLINE void MatrixInverse(void* _src, void* _dest) {
#if K3D_USE_SSE
            V4F *src = (V4F*)_src;
            V4F *dest = (V4F*)_dest;
            __m128 res_0 = _mm_shuffle_ps(src[0], src[1], _MM_PERM2(X, Y, X, Y));
            __m128 res_1 = _mm_shuffle_ps(src[0], src[1], _MM_PERM2(Z, W, Z, W));
            __m128 res_2 = _mm_shuffle_ps(src[2], src[3], _MM_PERM2(X, Y, X, Y));
            __m128 res_3 = _mm_shuffle_ps(src[2], src[3], _MM_PERM2(Z, W, Z, W));
            __m128 row_0 = _mm_shuffle_ps(res_0, res_2, _MM_PERM2(X, Z, X, Z));
            __m128 row_1 = _mm_shuffle_ps(res_2, res_0, _MM_PERM2(Y, W, Y, W));
            __m128 row_2 = _mm_shuffle_ps(res_1, res_3, _MM_PERM2(X, Z, X, Z));
            __m128 row_3 = _mm_shuffle_ps(res_3, res_1, _MM_PERM2(Y, W, Y, W));
            __m128 tmp_lane = _mm_mul_ps(row_2, row_3);
            tmp_lane = _MM_SWIZZLE(tmp_lane, Y, X, W, Z);
            res_0 = _mm_mul_ps(row_1, tmp_lane);
            res_1 = _mm_mul_ps(row_0, tmp_lane);
            tmp_lane = _MM_SWIZZLE(tmp_lane, Z, W, X, Y);
            res_0 = _mm_sub_ps(_mm_mul_ps(row_1, tmp_lane), res_0);
            res_1 = _mm_sub_ps(_mm_mul_ps(row_0, tmp_lane), res_1);
            res_1 = _MM_SWIZZLE(res_1, Z, W, X, Y);
            tmp_lane = _mm_mul_ps(row_1, row_2);
            tmp_lane = _MM_SWIZZLE(tmp_lane, Y, X, W, Z);
            res_0 = _mm_add_ps(_mm_mul_ps(row_3, tmp_lane), res_0);
            res_3 = _mm_mul_ps(row_0, tmp_lane);
            tmp_lane = _MM_SWIZZLE(tmp_lane, Z, W, X, Y);
            res_0 = _mm_sub_ps(res_0, _mm_mul_ps(row_3, tmp_lane));
            res_3 = _mm_sub_ps(_mm_mul_ps(row_0, tmp_lane), res_3);
            res_3 = _MM_SWIZZLE(res_3, Z, W, X, Y);
            tmp_lane = _mm_mul_ps(_MM_SWIZZLE(row_1, Z, W, X, Y), row_3);
            tmp_lane = _MM_SWIZZLE(tmp_lane, Y, X, W, Z);
            row_2 = _MM_SWIZZLE(row_2, Z, W, X, Y);
            res_0 = _mm_add_ps(_mm_mul_ps(row_2, tmp_lane), res_0);
            res_2 = _mm_mul_ps(row_0, tmp_lane);
            tmp_lane = _MM_SWIZZLE(tmp_lane, Z, W, X, Y);
            res_0 = _mm_sub_ps(res_0, _mm_mul_ps(row_2, tmp_lane));
            res_2 = _mm_sub_ps(_mm_mul_ps(row_0, tmp_lane), res_2);
            res_2 = _MM_SWIZZLE(res_2, Z, W, X, Y);
            tmp_lane = _mm_mul_ps(row_0, row_1);
            tmp_lane = _MM_SWIZZLE(tmp_lane, Y, X, W, Z);
            res_2 = _mm_add_ps(_mm_mul_ps(row_3, tmp_lane), res_2);
            res_3 = _mm_sub_ps(_mm_mul_ps(row_2, tmp_lane), res_3);
            tmp_lane = _MM_SWIZZLE(tmp_lane, Z, W, X, Y);
            res_2 = _mm_sub_ps(_mm_mul_ps(row_3, tmp_lane), res_2);
            res_3 = _mm_sub_ps(res_3, _mm_mul_ps(row_2, tmp_lane));
            tmp_lane = _mm_mul_ps(row_0, row_3);
            tmp_lane = _MM_SWIZZLE(tmp_lane, Y, X, W, Z);
            res_1 = _mm_sub_ps(res_1, _mm_mul_ps(row_2, tmp_lane));
            res_2 = _mm_add_ps(_mm_mul_ps(row_1, tmp_lane), res_2);
            tmp_lane = _MM_SWIZZLE(tmp_lane, Z, W, X, Y);
            res_1 = _mm_add_ps(_mm_mul_ps(row_2, tmp_lane), res_1);
            res_2 = _mm_sub_ps(res_2, _mm_mul_ps(row_1, tmp_lane));
            tmp_lane = _mm_mul_ps(row_0, row_2);
            tmp_lane = _MM_SWIZZLE(tmp_lane, Y, X, W, Z);
            res_1 = _mm_add_ps(_mm_mul_ps(row_3, tmp_lane), res_1);
            res_3 = _mm_sub_ps(res_3, _mm_mul_ps(row_1, tmp_lane));
            tmp_lane = _MM_SWIZZLE(tmp_lane, Z, W, X, Y);
            res_1 = _mm_sub_ps(res_1, _mm_mul_ps(row_3, tmp_lane));
            res_3 = _mm_add_ps(_mm_mul_ps(row_1, tmp_lane), res_3);
            __m128 det = _mm_mul_ps(row_0, res_0);
            det = _mm_add_ps(det, _MM_SWIZZLE(det, Y, X, W, Z));
            det = _mm_add_ss(det, _MM_SWIZZLE(det, Z, W, X, Y));
            tmp_lane = _MM_SWIZZLE(_mm_rcp_ss_nr(det), X, X, X, X);
            dest[0] = _mm_mul_ps(res_0, tmp_lane);
            dest[1] = _mm_mul_ps(res_1, tmp_lane);
            dest[2] = _mm_mul_ps(res_2, tmp_lane);
            dest[3] = _mm_mul_ps(res_3, tmp_lane);
#elif K3D_USE_NEON
#else

#endif
        }

        extern K3D_CORE_API void MemoryCopy(void* __restrict Dest, const void* __restrict Source, size_t NumQuadwords);
    }
}

#endif