#define USE_GLSL 0

#if USE_GLSL

#version 430
in vec4 vert;
uniform mat4 mvp;
out float intensity;
void main(void)
{
  intensity = vert.w;
  gl_Position = mvp * vec4(vert.xyz, 1.0);
}

#else

struct VS_IN
{
  float4 vert : POSITION;
};

cbuffer mat : register(b0)
{
    row_major matrix projectMatrix;
    row_major matrix modelMatrix;
    row_major matrix viewMatrix;   
};

struct VS_OUT
{
  float4 outPos : SV_POSITION;
  float intensity : INTENSITY;
};

VS_OUT main(VS_IN vs_in)
{
  VS_OUT vs_out;
  vs_out.outPos = projectMatrix * viewMatrix * modelMatrix * float4(vs_in.vert.xyz, 1.0);
  vs_out.intensity = vs_in.vert.w;
  return vs_out;
}

#endif