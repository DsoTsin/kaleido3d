/*******************************************************************************
 *
 * Maya_fixedFunction
 *
 *******************************************************************************/

cbuffer ConstantBuffer : register( b0 )
{
	// Matrices
	matrix wvIT : WorldViewInverseTranspose	: packoffset(c0);
	matrix wvp  : WorldViewProjection		: packoffset(c4);
	matrix wv   : WorldView					: packoffset(c8);
	
	// Material uniforms
	float3 lightDir			: packoffset(c12);
	float3 lightColor		: packoffset(c13);
	float3 ambientLight		: packoffset(c14);
	float3 diffuseMaterial	: packoffset(c15);
	float3 specularColor	: packoffset(c16);
	float  diffuseCoeff		: packoffset(c17.x);
	float  shininess		: packoffset(c17.y);
	float  transparency		: packoffset(c17.z);
}

// Material textured uniforms
Texture2D diffuseTexture : register( t0 );
SamplerState diffuseSampler : register( s0 );

struct VS_INPUT
{
	float4 vertex : POSITION;
	float3 normal : NORMAL;
	float2 tex : TEXCOORD0;
};

struct PS_INPUT
{
	float4 position: SV_POSITION;
	float3 eyeNormal : TEXCOORD0;
    float3 eyeView : TEXCOORD1;
    float2 texCoord : TEXCOORD2;
};

PS_INPUT mainVS( VS_INPUT input )
{
    PS_INPUT output = (PS_INPUT)0;

	output.position = mul( input.vertex, wvp);
    output.eyeNormal = mul( input.normal, (float3x3)wvIT);
	output.eyeView = -mul( input.vertex, wv).xyz;
	
	return output;
}

PS_INPUT mainTexVS( VS_INPUT input )
{ 
    PS_INPUT output = (PS_INPUT)0;

	output.position = mul( input.vertex, wvp);
    output.eyeNormal = mul( input.normal, (float3x3)wvIT);
	output.eyeView = -mul( input.vertex, wv).xyz;
    output.texCoord = input.tex.xy;
	
	return output;
}

float4 mainPS( PS_INPUT input ) : SV_Target
{
  
  //
  //	Since the EyeNormal is getting interpolated, we
  //	have to first restore it by normalizing it.
  //
  float3 norm = normalize( input.eyeNormal );
  float3 view = normalize( input.eyeView );
  float3 refl = normalize( -reflect(view,norm));

  float nDotL = saturate( dot( norm, lightDir));
  float rDotL = saturate( dot( refl, lightDir));
  rDotL = (nDotL > 0.0) ? pow( rDotL, shininess) : 0.0;

  float3 color = lightColor*nDotL*diffuseMaterial + diffuseMaterial*ambientLight + 
               rDotL*lightColor*specularColor;

  return float4( color, 1.0);

}

float4 mainTexPS( PS_INPUT input ) : SV_Target
{
  
  //
  //	Since the EyeNormal is getting interpolated, we
  //	have to first restore it by normalizing it.
  //
  float3 norm = normalize( input.eyeNormal );
  float3 view = normalize( input.eyeView );
  float3 refl = normalize( -reflect(view,norm));
  float3 diffuse = tex2D( diffuseSampler, input.texCoord ).rgb;

  float nDotL = saturate( dot( norm, lightDir));
  float rDotL = saturate( dot( refl, lightDir));
  rDotL = (nDotL > 0.0) ? pow( rDotL, shininess) : 0.0;

  float3 color = (lightColor * nDotL * diffuse * diffuseCoeff) + (diffuse * ambientLight) + 
					(rDotL * lightColor * specularColor);

  return float4( color, transparency);

}
