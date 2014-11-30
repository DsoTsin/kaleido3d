/*******************************************************************************
 *
 * Maya_unlit
 *
 *******************************************************************************/

cbuffer ConstantBuffer : register( b0 )
{
	matrix wvp : WorldViewProjection;
	float3 diffuseMaterial = float3( 0.8, 0.2, 0.0);
}

void mainVS( float4 vertex : POSITION,
             out float4 position: SV_POSITION )
{ 

	position = mul( vertex, wvp);
}

float4 mainPS() : SV_Target
{
 
  float3 color = diffuseMaterial;

  return float4( color, 1.0);

}
