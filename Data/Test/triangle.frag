#define USE_GLSL 0

#if USE_GLSL

#version 430

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 inColor;

layout (location = 0) out vec4 outFragColor;

void main()
{
  outFragColor = vec4(inColor, 1.0);
}

#else

struct VS_OUT
{
//    float4 outPos : SV_POSITION;
    float3 outColor: COLOR;
};

float4 main(VS_OUT psIn) : SV_TARGET {
    return float4(psIn.outColor, 1.0);
}

#endif