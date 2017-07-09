#define USE_GLSL 0

#if USE_GLSL

#version 400
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (binding = 0) uniform UBO 
{
	mat4 projectionMatrix;
	mat4 modelMatrix;
	mat4 viewMatrix;
} ubo;

layout (location = 0) in vec3 pos;
layout (location = 1) in vec4 vertColor;
layout (location = 2) in vec2 vertUV;

layout (location = 0) out vec4 color;
layout (location = 1) out vec2 uv;

void main() {
   color = vertColor;
   uv = vertUV;
   gl_Position  = ubo.projectionMatrix * ubo.viewMatrix * ubo.modelMatrix * vec4(pos, 1.0f);
}

#else

cbuffer UBO : register(b0)
{
	row_major matrix projectMatrix;
	row_major matrix modelMatrix;
	row_major matrix viewMatrix;
};

struct VS_IN
{
    float3 pos : POSITION;
    float4 vertColor: COLOR;
    float2 vertUV: TEXCOORD0;
};

struct VS_OUT
{
    float4 outPos: SV_POSITION;
    float4 outColor: COLOR;
    float2 outCoord: TEXCOORD0;
};

VS_OUT main(VS_IN vs_in)
{
    VS_OUT vs_out;
    vs_out.outPos = projectMatrix * viewMatrix * modelMatrix * float4(vs_in.pos, 1.0);
    vs_out.outColor = vs_in.vertColor;
    vs_out.outCoord = vs_in.vertUV;
    return vs_out;
}


#endif