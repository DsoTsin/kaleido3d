#include "common.hlsl"

cbuffer cbPerObjectVS : register( b0 ) // Model vertex shader constants
{
    float4x4 WorldViewProjection	: packoffset( c0 );
	float4x4 World					: packoffset( c4 );
}

cbuffer cbPerObjectPS : register( b0 ) // Model pixel shader constants
{
	float specExp		: packoffset( c0 );
	float specIntensity	: packoffset( c0.y );
}

Texture2D    DiffuseTexture	: register( t0 );
SamplerState LinearSampler	: register( s0 );

struct VS_INPUT
{
    float4 Position	: POSITION;		// vertex position 
    float3 Normal	: NORMAL;		// vertex normal
    float2 UV		: TEXCOORD0;	// vertex texture coords 
};

struct VS_OUTPUT
{
    float4 Position	: SV_POSITION;	// vertex position 
    float2 UV		: TEXCOORD0;	// vertex texture coords
	float3 Normal	: TEXCOORD1;	// vertex normal
};


VS_OUTPUT RenderSceneVS( VS_INPUT input )
{
    VS_OUTPUT Output;
    float3 vNormalWorldSpace;
    
    // Transform the position from object space to homogeneous projection space
    Output.Position = mul( input.Position, WorldViewProjection );

    // Just copy the texture coordinate through
    Output.UV = input.UV; 

	// Transform the normal to world space
	Output.Normal = mul(input.Normal, (float3x3)World);
    
    return Output;    
}

struct PS_GBUFFER_OUT
{
	float4 ColorSpecInt : SV_TARGET0;
	float4 Normal : SV_TARGET1;
	float4 SpecPow : SV_TARGET2;
};

PS_GBUFFER_OUT PackGBuffer(float3 BaseColor, float3 Normal, float SpecIntensity, float SpecPower)
{
	PS_GBUFFER_OUT Out;
	float SpecPowerNorm = max(0.0001, (SpecPower - g_SpecPowerRange.x) / g_SpecPowerRange.y);

	Out.ColorSpecInt = float4(BaseColor.rgb, SpecIntensity);
	Out.Normal = float4(Normal * 0.5 + 0.5, 0.0);
	Out.SpecPow = float4(SpecPowerNorm, 0.0, 0.0, 0.0);

	return Out;
}

PS_GBUFFER_OUT RenderScenePS( VS_OUTPUT In )
{ 
    float3 DiffuseColor = DiffuseTexture.Sample( LinearSampler, In.UV );
	DiffuseColor *= DiffuseColor;

	return PackGBuffer(DiffuseColor, normalize(In.Normal), specIntensity, specExp);
}
