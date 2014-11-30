#pragma once
#ifndef __kMath_NEON_hpp__
#define __kMath_NEON_hpp__
#include <Config/Config.h>
#include <arm_neon.h>
typedef float32x4_t vec4x32;

KFORCE_INLINE vec4x32 simd_add(vec4x32 & a, vec4x32 & b) {
	return vaddq_f32(a, b);
}

KFORCE_INLINE vec4x32 simd_dot(vec4x32 const & a, vec4x32 const & b) {
	vec4x32 dot = vmulq_f32(a, b);
	float32x2_t inter = vpadd_f32(vget_low_f32(dot), vget_high_f32(dot));
	float32x2_t inter2 = vpadd_f32(inter, inter);
	return vcombine_f32(inter2, inter2);
}

KFORCE_INLINE vec4x32 simd_mul(vec4x32 const & a, vec4x32 const & b) {
	return vmulq_f32(a, b);
}

KFORCE_INLINE vec4x32 simd_set(float v) {
	return vdupq_n_f32(v);
}

KFORCE_INLINE vec4x32 simd_set(float x, float y, float z, float w) {
	const vec4x32 r = { x, y, z, w };
	return r;
}

KFORCE_INLINE vec4x32 simd_reciprocal_sqrt(vec4x32 const & v) {
	return vrsqrteq_f32(v);
}

KFORCE_INLINE vec4x32 simd_reciprocal_length(vec4x32 const & v) {
	vec4x32 & recip = simd_dot(v, v);
	return simd_reciprocal_sqrt(recip);
}

KFORCE_INLINE vec4x32 simd_reciprocal(vec4x32 const & v) {
	return vrecpeq_f32(v);
}

KFORCE_INLINE vec4x32 simd_normalize(vec4x32 const & v) {
	return simd_mul(v, simd_reciprocal_length(v));
}

KFORCE_INLINE void simd_matrix4_mul(void* result, void* a, void* b) {
	const vec4x32 *in1 = (const vec4x32 *)a;
	const vec4x32 *in2 = (const vec4x32 *)b;
	vec4x32 *out = (vec4x32 *)result;
	vec4x32 inter;
	{
		inter = vmulq_lane_f32(in2[0], vget_low_f32(in1[0]), 0);
		inter = vmlaq_lane_f32(inter, in2[1], vget_low_f32(in1[0]), 1);
		inter = vmlaq_lane_f32(inter, in2[2], vget_high_f32(in1[0]), 0);
		out[0] = vmlaq_lane_f32(inter, in2[3], vget_high_f32(in1[0]), 1);
	}

	{
		inter = vmulq_lane_f32(in2[0], vget_low_f32(in1[1]), 0);
		inter = vmlaq_lane_f32(inter, in2[1], vget_low_f32(in1[1]), 1);
		inter = vmlaq_lane_f32(inter, in2[2], vget_high_f32(in1[1]), 0);
		out[1] = vmlaq_lane_f32(inter, in2[3], vget_high_f32(in1[1]), 1);
	}

	{
		inter = vmulq_lane_f32(in2[0], vget_low_f32(in1[2]), 0);
		inter = vmlaq_lane_f32(inter, in2[1], vget_low_f32(in1[2]), 1);
		inter = vmlaq_lane_f32(inter, in2[2], vget_high_f32(in1[2]), 0);
		out[2] = vmlaq_lane_f32(inter, in2[3], vget_high_f32(in1[2]), 1);
	}

	{
		inter = vmulq_lane_f32(in2[0], vget_low_f32(in1[3]), 0);
		inter = vmlaq_lane_f32(inter, in2[1], vget_low_f32(in1[3]), 1);
		inter = vmlaq_lane_f32(inter, in2[2], vget_high_f32(in1[3]), 0);
		out[3] = vmlaq_lane_f32(inter, in2[3], vget_high_f32(in1[3]), 1);
	}
}

#endif