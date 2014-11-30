//
// Shaders
//

struct Matrix
{
	matrix mat;
};

cbuffer worldMatrix : register(b1)
{
	matrix mat;
};

//ConstantBuffer<Matrix> worldMatrix : register(b0);
//ConstantBuffer<Matrix> viewMatrix : register(b1);
//ConstantBuffer<Matrix> projMatrix : register(b2);

cbuffer worldMatrix : register(b0)
{
	matrix worldmat;
};

cbuffer viewMatrix : register(b1)
{
	matrix viewmat;
};

cbuffer projMatrix : register(b2)
{
	matrix projmat;
};


Texture2D<float4> tex0 : register(t0);
SamplerState samp0 : register(s0);

struct VSOutput
{
	float4 pos : SV_POSITION;
	float2 tex : TEXCOORD;
};

VSOutput VSMain(float3 pos : POSITION, float2 tex : TEXCOORD)
{
	VSOutput vsOut;
	vsOut.pos = mul(float4(pos, 1.0), worldmat);
	vsOut.pos = mul(vsOut.pos, viewmat);
	vsOut.pos = mul(vsOut.pos, projmat);
	vsOut.tex = tex;
	return vsOut;
}

float4 PSMain(VSOutput vsOut) : SV_Target
{
	float4 outColor;
	outColor = tex0.Sample(samp0, vsOut.tex.xy);
	return outColor;
	//return vsOut.color;
}
