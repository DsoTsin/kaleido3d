#define USE_GLSL 0

#if USE_GLSL

#version 430 
layout (location = 0) out vec4 color;
in float intensity;
void main(void)
{
    color = mix(vec4(0.0f, 0.2f, 1.0f, 1.0f), vec4(0.2f, 0.05f, 0.0f, 1.0f), intensity);
}

#else

struct PS_IN
{
	float intensity : INTENSITY;	
};

float4 main(PS_IN ps_in) : SV_Target
{
	return lerp(float4(0.0f, 0.2f, 1.0f, 1.0f), float4(0.2f, 0.05f, 0.0f, 1.0f), ps_in.intensity);
}

#endif