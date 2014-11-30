#include "common.hlsl"

cbuffer cbDirLight : register( b1 )
{
	float3 AmbientDown	: packoffset( c0 );
	float3 AmbientRange	: packoffset( c1 );
	float3 DirToLight		: packoffset( c2 );
	float3 DirLightColor	: packoffset( c3 );
}

static const float2 arrBasePos[4] = {
	float2(-1.0, 1.0),
	float2(1.0, 1.0),
	float2(-1.0, -1.0),
	float2(1.0, -1.0),
};

struct VS_OUTPUT
{
	float4 Position : SV_Position;
	float2 cpPos	: TEXCOORD0;
};

VS_OUTPUT DirLightVS( uint VertexID : SV_VertexID )
{
	VS_OUTPUT Output;

	Output.Position = float4( arrBasePos[VertexID].xy, 0.0, 1.0);
	Output.cpPos = Output.Position.xy;

	return Output;    
}

float3 CalcAmbient(float3 normal, float3 color)
{
	float up = normal.y * 0.5 + 0.5;
	float3 ambient = AmbientDown + up * AmbientRange;
	return ambient * color;
}

float3 CalcDirectional(float3 position, Material material)
{
	// Phong diffuse
	float NDotL = dot(DirToLight, material.normal);
	float3 finalColor = DirLightColor.rgb * saturate(NDotL);
   
	// Blinn specular
	float3 ToEye = EyePosition - position;
	ToEye = normalize(ToEye);
	float3 HalfWay = normalize(ToEye + DirToLight);
	float NDotH = saturate(dot(HalfWay, material.normal));
	finalColor += DirLightColor.rgb * pow(NDotH, material.specPow) * material.specIntensity;

	return finalColor * material.diffuseColor.rgb;
}

float4 DirLightPS(VS_OUTPUT In) : SV_TARGET
{
	SURFACE_DATA gbd = UnpackGBuffer_Loc(In.Position.xy);
	Material mat;
	MaterialFromGBuffer(gbd, mat);

	float3 position = CalcWorldPos(In.cpPos, gbd.LinearDepth);
	float3 finalColor = CalcAmbient(mat.normal, mat.diffuseColor.rgb);
	finalColor += CalcDirectional(position, mat);
	return float4(finalColor, 1.0);
}
