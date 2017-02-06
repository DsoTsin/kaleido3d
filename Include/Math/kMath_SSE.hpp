#pragma once
#ifndef __kMath_SSE_hpp__
#define __kMath_SSE_hpp__

#include "../Config/Config.h"
#include <xmmintrin.h>

#define _MM_PERM2_X			0
#define _MM_PERM2_Y			1
#define _MM_PERM2_Z			2
#define _MM_PERM2_W			3
#define _MM_PERM2(X,Y,Z,W)	_MM_SHUFFLE(_MM_PERM2_ ## W,_MM_PERM2_ ## Z,_MM_PERM2_ ## Y,_MM_PERM2_ ## X)
#define _MM_SWIZZLE(V,X,Y,Z,W) _mm_shuffle_ps(V,V,_MM_PERM2(X,Y,Z,W))
typedef __m128 vec4x32;

KFORCE_INLINE vec4x32 _mm_rcp_ss_nr(vec4x32 v) {
	vec4x32 iv = _mm_rcp_ss(v);
	return _mm_sub_ss(_mm_add_ss(iv, iv), _mm_mul_ss(v, _mm_mul_ss(iv, iv)));
}

KFORCE_INLINE vec4x32 simd_add(vec4x32 & a, vec4x32 & b) {
	return _mm_add_ps(a, b);
}

KFORCE_INLINE vec4x32 simd_dot(vec4x32 const & a, vec4x32 const & b) {
	vec4x32 dot = _mm_mul_ps(a, b);
	vec4x32 tmp = _mm_shuffle_ps(dot, dot, _MM_SHUFFLE(0, 0, 3, 2));
	dot = _mm_add_ps(dot, tmp);
	tmp = _mm_shuffle_ps(dot, dot, _MM_SHUFFLE(0, 0, 0, 1));
	return _mm_add_ps(dot, tmp);
}

KFORCE_INLINE vec4x32 simd_mul(vec4x32 const & a, vec4x32 const & b) {
	return _mm_mul_ps(a, b);
}

KFORCE_INLINE vec4x32 simd_set(float v) {
	return _mm_set_ps1(v);
}

KFORCE_INLINE vec4x32 simd_set(float x, float y, float z, float w) {
	return _mm_setr_ps(x, y, z, w);
}

KFORCE_INLINE vec4x32 simd_reciprocal_sqrt(vec4x32 const & v) {
	return _mm_rsqrt_ps(v);
}

KFORCE_INLINE vec4x32 simd_reciprocal_length(vec4x32 const & v) {
	const vec4x32 & recip = simd_dot(v, v);
	return simd_reciprocal_sqrt(recip);
}

KFORCE_INLINE vec4x32 simd_reciprocal(vec4x32 const & v) {
	return _mm_rcp_ps(v);
}

KFORCE_INLINE vec4x32 simd_normalize(vec4x32 const & v) {
	return simd_mul(v, simd_reciprocal_length(v));
}

KFORCE_INLINE void simd_matrix4_mul(void* result, void* a, void* b) {
	vec4x32 *in1 = (vec4x32*)a;
	vec4x32 *in2 = (vec4x32*)b;
	vec4x32 *out = (vec4x32*)result;
	{
		vec4x32 e0 = _mm_shuffle_ps(in2[0], in2[0], _MM_SHUFFLE(0, 0, 0, 0));
		vec4x32 e1 = _mm_shuffle_ps(in2[0], in2[0], _MM_SHUFFLE(1, 1, 1, 1));
		vec4x32 e2 = _mm_shuffle_ps(in2[0], in2[0], _MM_SHUFFLE(2, 2, 2, 2));
		vec4x32 e3 = _mm_shuffle_ps(in2[0], in2[0], _MM_SHUFFLE(3, 3, 3, 3));

		vec4x32 m0 = _mm_mul_ps(in1[0], e0);
		vec4x32 m1 = _mm_mul_ps(in1[1], e1);
		vec4x32 m2 = _mm_mul_ps(in1[2], e2);
		vec4x32 m3 = _mm_mul_ps(in1[3], e3);

		vec4x32 a0 = _mm_add_ps(m0, m1);
		vec4x32 a1 = _mm_add_ps(m2, m3);
		vec4x32 a2 = _mm_add_ps(a0, a1);

		out[0] = a2;
	}

	{
		vec4x32 e0 = _mm_shuffle_ps(in2[1], in2[1], _MM_SHUFFLE(0, 0, 0, 0));
		vec4x32 e1 = _mm_shuffle_ps(in2[1], in2[1], _MM_SHUFFLE(1, 1, 1, 1));
		vec4x32 e2 = _mm_shuffle_ps(in2[1], in2[1], _MM_SHUFFLE(2, 2, 2, 2));
		vec4x32 e3 = _mm_shuffle_ps(in2[1], in2[1], _MM_SHUFFLE(3, 3, 3, 3));

		vec4x32 m0 = _mm_mul_ps(in1[0], e0);
		vec4x32 m1 = _mm_mul_ps(in1[1], e1);
		vec4x32 m2 = _mm_mul_ps(in1[2], e2);
		vec4x32 m3 = _mm_mul_ps(in1[3], e3);

		vec4x32 a0 = _mm_add_ps(m0, m1);
		vec4x32 a1 = _mm_add_ps(m2, m3);
		vec4x32 a2 = _mm_add_ps(a0, a1);

		out[1] = a2;
	}

	{
		vec4x32 e0 = _mm_shuffle_ps(in2[2], in2[2], _MM_SHUFFLE(0, 0, 0, 0));
		vec4x32 e1 = _mm_shuffle_ps(in2[2], in2[2], _MM_SHUFFLE(1, 1, 1, 1));
		vec4x32 e2 = _mm_shuffle_ps(in2[2], in2[2], _MM_SHUFFLE(2, 2, 2, 2));
		vec4x32 e3 = _mm_shuffle_ps(in2[2], in2[2], _MM_SHUFFLE(3, 3, 3, 3));

		vec4x32 m0 = _mm_mul_ps(in1[0], e0);
		vec4x32 m1 = _mm_mul_ps(in1[1], e1);
		vec4x32 m2 = _mm_mul_ps(in1[2], e2);
		vec4x32 m3 = _mm_mul_ps(in1[3], e3);

		vec4x32 a0 = _mm_add_ps(m0, m1);
		vec4x32 a1 = _mm_add_ps(m2, m3);
		vec4x32 a2 = _mm_add_ps(a0, a1);

		out[2] = a2;
	}

	{
		vec4x32 e0 = _mm_shuffle_ps(in2[3], in2[3], _MM_SHUFFLE(0, 0, 0, 0));
		vec4x32 e1 = _mm_shuffle_ps(in2[3], in2[3], _MM_SHUFFLE(1, 1, 1, 1));
		vec4x32 e2 = _mm_shuffle_ps(in2[3], in2[3], _MM_SHUFFLE(2, 2, 2, 2));
		vec4x32 e3 = _mm_shuffle_ps(in2[3], in2[3], _MM_SHUFFLE(3, 3, 3, 3));

		vec4x32 m0 = _mm_mul_ps(in1[0], e0);
		vec4x32 m1 = _mm_mul_ps(in1[1], e1);
		vec4x32 m2 = _mm_mul_ps(in1[2], e2);
		vec4x32 m3 = _mm_mul_ps(in1[3], e3);

		vec4x32 a0 = _mm_add_ps(m0, m1);
		vec4x32 a1 = _mm_add_ps(m2, m3);
		vec4x32 a2 = _mm_add_ps(a0, a1);

		out[3] = a2;
	}
}

KFORCE_INLINE void simd_matrix4_inverse(void* _src, void* _dest) {
	vec4x32 *src = (vec4x32*)_src;
	vec4x32 *dest = (vec4x32*)_dest;
	__m128 res_0 = _mm_shuffle_ps(src[0], src[1], _MM_PERM2(X, Y, X, Y));
	__m128 res_1 = _mm_shuffle_ps(src[0], src[1], _MM_PERM2(Z, W, Z, W));
	__m128 res_2 = _mm_shuffle_ps(src[2], src[3], _MM_PERM2(X, Y, X, Y));
	__m128 res_3 = _mm_shuffle_ps(src[2], src[3], _MM_PERM2(Z, W, Z, W));
	__m128 row_0 = _mm_shuffle_ps(res_0, res_2, _MM_PERM2(X, Z, X, Z));
	__m128 row_1 = _mm_shuffle_ps(res_2, res_0, _MM_PERM2(Y, W, Y, W));
	__m128 row_2 = _mm_shuffle_ps(res_1, res_3, _MM_PERM2(X, Z, X, Z));
	__m128 row_3 = _mm_shuffle_ps(res_3, res_1, _MM_PERM2(Y, W, Y, W));
	__m128 temp = _mm_mul_ps(row_2, row_3);
	temp = _MM_SWIZZLE(temp, Y, X, W, Z);
	res_0 = _mm_mul_ps(row_1, temp);
	res_1 = _mm_mul_ps(row_0, temp);
	temp = _MM_SWIZZLE(temp, Z, W, X, Y);
	res_0 = _mm_sub_ps(_mm_mul_ps(row_1, temp), res_0);
	res_1 = _mm_sub_ps(_mm_mul_ps(row_0, temp), res_1);
	res_1 = _MM_SWIZZLE(res_1, Z, W, X, Y);
	temp = _mm_mul_ps(row_1, row_2);
	temp = _MM_SWIZZLE(temp, Y, X, W, Z);
	res_0 = _mm_add_ps(_mm_mul_ps(row_3, temp), res_0);
	res_3 = _mm_mul_ps(row_0, temp);
	temp = _MM_SWIZZLE(temp, Z, W, X, Y);
	res_0 = _mm_sub_ps(res_0, _mm_mul_ps(row_3, temp));
	res_3 = _mm_sub_ps(_mm_mul_ps(row_0, temp), res_3);
	res_3 = _MM_SWIZZLE(res_3, Z, W, X, Y);
	temp = _mm_mul_ps(_MM_SWIZZLE(row_1, Z, W, X, Y), row_3);
	temp = _MM_SWIZZLE(temp, Y, X, W, Z);
	row_2 = _MM_SWIZZLE(row_2, Z, W, X, Y);
	res_0 = _mm_add_ps(_mm_mul_ps(row_2, temp), res_0);
	res_2 = _mm_mul_ps(row_0, temp);
	temp = _MM_SWIZZLE(temp, Z, W, X, Y);
	res_0 = _mm_sub_ps(res_0, _mm_mul_ps(row_2, temp));
	res_2 = _mm_sub_ps(_mm_mul_ps(row_0, temp), res_2);
	res_2 = _MM_SWIZZLE(res_2, Z, W, X, Y);
	temp = _mm_mul_ps(row_0, row_1);
	temp = _MM_SWIZZLE(temp, Y, X, W, Z);
	res_2 = _mm_add_ps(_mm_mul_ps(row_3, temp), res_2);
	res_3 = _mm_sub_ps(_mm_mul_ps(row_2, temp), res_3);
	temp = _MM_SWIZZLE(temp, Z, W, X, Y);
	res_2 = _mm_sub_ps(_mm_mul_ps(row_3, temp), res_2);
	res_3 = _mm_sub_ps(res_3, _mm_mul_ps(row_2, temp));
	temp = _mm_mul_ps(row_0, row_3);
	temp = _MM_SWIZZLE(temp, Y, X, W, Z);
	res_1 = _mm_sub_ps(res_1, _mm_mul_ps(row_2, temp));
	res_2 = _mm_add_ps(_mm_mul_ps(row_1, temp), res_2);
	temp = _MM_SWIZZLE(temp, Z, W, X, Y);
	res_1 = _mm_add_ps(_mm_mul_ps(row_2, temp), res_1);
	res_2 = _mm_sub_ps(res_2, _mm_mul_ps(row_1, temp));
	temp = _mm_mul_ps(row_0, row_2);
	temp = _MM_SWIZZLE(temp, Y, X, W, Z);
	res_1 = _mm_add_ps(_mm_mul_ps(row_3, temp), res_1);
	res_3 = _mm_sub_ps(res_3, _mm_mul_ps(row_1, temp));
	temp = _MM_SWIZZLE(temp, Z, W, X, Y);
	res_1 = _mm_sub_ps(res_1, _mm_mul_ps(row_3, temp));
	res_3 = _mm_add_ps(_mm_mul_ps(row_1, temp), res_3);
	__m128 det = _mm_mul_ps(row_0, res_0);
	det = _mm_add_ps(det, _MM_SWIZZLE(det, Y, X, W, Z));
	det = _mm_add_ss(det, _MM_SWIZZLE(det, Z, W, X, Y));
	temp = _MM_SWIZZLE(_mm_rcp_ss_nr(det), X, X, X, X);
	dest[0] = _mm_mul_ps(res_0, temp);
	dest[1] = _mm_mul_ps(res_1, temp);
	dest[2] = _mm_mul_ps(res_2, temp);
	dest[3] = _mm_mul_ps(res_3, temp);
}

#endif