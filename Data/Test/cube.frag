#define USE_GLSL 0
#if USE_GLSL
#version 400
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (set = 0, binding = 1) uniform sampler2D tex;

layout (location = 0) in vec4 color;
layout (location = 1) in vec2 uv;

layout (location = 0) out vec4 uFragColor;
void main() {
  uFragColor = texture(tex, uv) * color;
}
#else

SamplerState  samp : register(s1);
Texture2D<float4> tex : register(t1);

struct PS_IN
{
//    float4 outPos: SV_POSITION;
    float4 outColor: COLOR;
    float2 outCoord: TEXCOORD0;
};

float4 main(PS_IN ps_in) : SV_TARGET
{
    return tex.Sample(samp, ps_in.outCoord);
}

#endif