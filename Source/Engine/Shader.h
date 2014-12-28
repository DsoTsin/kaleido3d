#pragma once 
#include <Core/Archive.h>
#include <Core/Variant.h>

namespace k3d {

	enum class ShaderType {
		Vertex,
		Pixel,
		Domain,
		Hull,
		Geometry,
		Compute
	};

	class Shader {
	public:
		virtual ~Shader() {}

	};

	template <typename ShaderInstClass, ShaderType ShaderInstType>
	class TShader : public Shader {
	public:
		static const ShaderType type = ShaderInstType;

		friend Archive & operator << (Archive & ar, const TShader & shader)
		{
			assert(shader.m_Blob.Size() != 0);
			ar << shader.m_Blob;
			return ar;
		}
		
		ShaderInstClass Get() {
			return ShaderInst;
		}

	protected:
		Variant m_Blob; // dx11 shader needed
		typename ShaderInstClass	ShaderInst;
	};

}