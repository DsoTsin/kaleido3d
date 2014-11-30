
cbuffer cbPerObjectVS : register(b0)
{
	float4x4 WorldViewProjection	: packoffset(c0);
	float4x4 World					: packoffset(c4);
}

cbuffer cbPerObjectPS : register(b0)
{
	float3 EyePosition	: packoffset(c0);
	float specExp : packoffset(c0.w);
	float specIntensity : packoffset(c1);
}

cbuffer PointLightConstants : register(b1)
{
	float3 PointLightPosition	: packoffset(c0);
	float PointLightRangeRcp : packoffset(c0.w);
	float3 PointLightColor	: packoffset(c1)
}

struct VS_INPUT
{
	float4 Position : POSITION;
	float3 Normal	: NORMAL;
};

struct VS_OUTPUT
{
	float4 Position : SV_POSITION;
	float3 Normal;
};

struct Material
{
	float3 normal;
	float4 diffuse;
	float specExp;
	float specIntensity;
};

float3 CalculatePointLighting(float3 position, Material material)
{
	float3 ToLight = PointLightPosition.xyz - position;
	float3 ToEye = EyePosition.xyz - position;
	float DistToLight = length(ToLight);
}

VS_OUTPUT RenderSceneVS(VS_INPUT input)
{
	VS_OUTPUT output;
	float3 normalWorldSpace;
	output.Position = mul(input.Position, WorldViewProjection);

	return output;
}

float4 RenderScenePS(VS_OUTPUT In) : SV_TARGET0
{

}