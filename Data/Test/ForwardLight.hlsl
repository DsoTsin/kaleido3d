/////////////////////////////////////////////////////////////////////////////
// Constant Buffers
/////////////////////////////////////////////////////////////////////////////
cbuffer cbPerObjectVS : register( b0 ) // Model constants
{
    matrix  WorldViewProjection  : packoffset( c0 );
	matrix  World				: packoffset( c4 );
}

cbuffer cbDirLightPS : register( b1 ) // Ambient light constants
{
	float3 AmbientDown	: packoffset( c0 );
	float3 AmbientRange	: packoffset( c1 );
}

/////////////////////////////////////////////////////////////////////////////
// Diffuse texture and linear sampler
/////////////////////////////////////////////////////////////////////////////
Texture2D    DiffuseTexture : register( t0 );
SamplerState LinearSampler : register( s0 );

/////////////////////////////////////////////////////////////////////////////
// shader input/output structure
/////////////////////////////////////////////////////////////////////////////
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

/////////////////////////////////////////////////////////////////////////////
// Vertex shader
/////////////////////////////////////////////////////////////////////////////
VS_OUTPUT RenderSceneVS( VS_INPUT input )
{
    VS_OUTPUT Output;
    float3 vNormalWorldSpace;
    
    // Transform the position from object space to homogeneous projection space
    Output.Position = mul( input.Position, WorldViewProjection );
    
    // Just copy the texture coordinate through
    Output.UV = input.UV; 

	Output.Normal = mul(input.Normal, (float3x3)World);
    
    return Output;    
}

/////////////////////////////////////////////////////////////////////////////
// Pixel shaders
/////////////////////////////////////////////////////////////////////////////

// Ambient calculation helper function
float3 CalcAmbient(float3 normal, float3 color)
{
	// Convert from [-1, 1] to [0, 1]
	float up = normal.y * 0.5 + 0.5;

	// Calculate the ambient value
	float3 ambient = AmbientDown + up * AmbientRange;

	// Apply the ambient value to the color
	return ambient * color;
}

float4 AmbientLightPS( VS_OUTPUT In ) : SV_TARGET0
{ 
    // Sample the texture and convert to linear space
    float3 DiffuseColor =  DiffuseTexture.Sample( LinearSampler, In.UV ).rgb;
	DiffuseColor *= DiffuseColor;

	// Calculate the ambient color
	float3 AmbientColor = CalcAmbient(In.Normal, DiffuseColor);

	// Return the ambient color
	return float4(AmbientColor, 1.0);
}