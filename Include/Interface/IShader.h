#pragma once

enum class ShaderType {
	Vertex,
	Pixel,
	Domain,
	Hull,
	Geometry,
	Compute
};


struct IShader {
public:
	virtual ~IShader() {}

	virtual void SaveCache() {}
	virtual void LoadCache() {}
};


template <typename ShaderInstClass, ShaderType ShaderInstType>
class TShader
{
public:
	static const ShaderType		type = ShaderInstType;
	ShaderInstClass				Get() { return ShaderInst; }
protected:
    ShaderInstClass             ShaderInst;
};