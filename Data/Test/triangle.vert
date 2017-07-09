#define USE_GLSL 0

#if USE_GLSL

#version 430

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inColor;

layout (binding = 0) uniform UBO
{
	mat4 projectionMatrix;
	mat4 modelMatrix;
	mat4 viewMatrix;
} ubo;

layout (location = 0) out vec3 outColor;

out gl_PerVertex
{
    vec4 gl_Position;
};


void main()
{
	outColor = inColor;
	gl_Position = ubo.projectionMatrix * ubo.viewMatrix * ubo.modelMatrix * vec4(inPos.xyz, 1.0);
}

#else

#pragma pack_matrix( row_major )

struct VS_IN
{
    float3 inPos : POSITION;
    float3 inColor : COLOR;
};

struct VS_OUT
{
    float4 outPos   : SV_POSITION;
    float3 outColor : COLOR;
};

cbuffer UBO : register(b0)
{
    row_major matrix projectMatrix;
    row_major matrix modelMatrix;
    row_major matrix viewMatrix;
};

VS_OUT main(VS_IN vsin)
{
    VS_OUT vsout;
    vsout.outPos = projectMatrix * viewMatrix * modelMatrix * float4(vsin.inPos.xyz, 1.0);
    vsout.outColor = vsin.inColor;
    return vsout;
}
#endif