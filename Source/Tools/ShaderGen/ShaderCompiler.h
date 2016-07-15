#ifndef __ShaderGen_h__
#define __ShaderGen_h__

#include <RHI/IRHIDefs.h>
#include <Config/Config.h>
#include <string>

namespace k3d 
{
	namespace shaderbinding
	{
		enum class ESemantic
		{
			EPOSITION,
			ECOLOR,
			ENORMAL,
			ENumSemanics
		};

		enum class EDataType
		{
			EBool, EBool2, EBool3, EBool4,
			EInt, EInt2, EInt3, EInt4,
			EUInt, EUInt2, EUInt3, EUInt4,
			EFloat, EFloat2, EFloat3, EFloat4,
			EMat2, EMat2x3, EMat2x4, EMat3x2, EMat3, EMat3x4, EMat4x2, EMat4x3, EMat4,
			EUnknown
		};

		enum class EBindType
		{
			EUndefined		= 0,
			EBlock			= 0x1,
			ESampler		= 0x1 << 1,
			EStorageImage	= 0x1 << 2,
			EStorageBuffer	= 0x1 << 3,
			EConstants		= 0x000000010
		};

		struct Attribute
		{
			std::string	VarName;
			ESemantic	VarSemantic;
			EDataType	VarType;
			uint32		VarLocation;
			uint32		VarBindingPoint;
			uint32		VarCount;
		};

		struct Uniform
		{
			Uniform() : VarType(EDataType::EUnknown), VarOffset(0), VarName{0}, VarSzArray(0) {}
			
			Uniform(EDataType type, std::string const & name, uint32 offset, uint32 szArray=0) 
				: VarName{0}, VarType(type)
				, VarOffset(offset), VarSzArray(szArray)
			{
				size_t name_len = name.length();
				strncpy(VarName, name.c_str(), name_len>128?128:name_len);
			}
			
			virtual ~Uniform() {}

			EDataType	VarType;
			char 		VarName[128];
			uint32		VarOffset;
			uint32		VarSzArray;
		};

		class Constant : public Uniform 
		{
		public:

			Constant() {}
			~Constant() override {}

		};

		struct Binding
		{
			Binding() : VarType(EBindType::EUndefined), VarName{0}, VarStage(rhi::ES_Vertex), VarNumber(0) {}
			Binding(EBindType t, std::string n, rhi::EShaderType st, uint32 num)
					: VarType(t), VarName{0}, VarStage(st), VarNumber(num)
			{
				size_t name_len = n.length();
				strncpy(VarName, n.c_str(), name_len>128?128:name_len);
			}

			EBindType			VarType;
			char				VarName[128];
			rhi::EShaderType	VarStage;
			uint32				VarNumber;

			Binding &operator = (Binding const &rhs)
			{
				VarType = rhs.VarType;
				strncpy(VarName, rhs.VarName, 128);
				VarStage = rhs.VarStage;
				VarNumber = rhs.VarNumber;
				return *this;
			}
		};
		
		struct Set
		{
			typedef uint32 VarIndex;
		};

		struct BindingTable
		{
			DynArray<Binding>		Bindings;
			DynArray<Uniform>		Uniforms;
			DynArray<Set::VarIndex>	Sets;

			BindingTable& AddBinding(Binding && binding)
			{
				this->Bindings.Append(binding);
				return *this;
			}

			BindingTable& AddUniform(Uniform && uniform)
			{
				this->Uniforms.Append(uniform);
				return *this;
			}

			BindingTable& AddSet(Set::VarIndex const& set) 
			{
				this->Sets.Append(set);
				return *this;
			}
		};
	}

	class Shader
	{
	public:
		enum 
		{
			EBinary,
			ESource
		};

		Shader() : m_SrcType(EBinary), m_Type(rhi::EShaderType::ShaderTypeNum) {}

		Shader(std::string const & source, rhi::EShaderType type) 
			: m_SrcType(ESource), m_Type(type), m_Source(source) 
		{
		}
		
		virtual ~Shader() {}

	protected:
		uint32				m_SrcType;
		rhi::EShaderType	m_Type;
		std::string			m_Source;
		rhi::ShaderByteCode m_ByteCodes;
	};
	
	class IShaderCompilerOutput
	{
	public:
		using Attributes = std::vector<shaderbinding::Attribute>;
		virtual const char*					GetErrorMsg() const = 0;
		virtual const char*					GetShaderBytes() const = 0;
		virtual const uint32				GetByteCount() const = 0;
		virtual const rhi::ShaderByteCode&	GetByteCode() const = 0;
		virtual shaderbinding::BindingTable GetBindingTable() const = 0;
		virtual const Attributes &			GetAttributes() const = 0;
		virtual								~IShaderCompilerOutput() {}
	};

	enum class EShaderLang
	{
		HLSL,
		GLSL,
		MetalSL
	};

	enum class EShaderCompileResult
	{
		Success,
		Fail
	};
	
	struct ShaderCompilerOption
	{
		rhi::EShaderType	ShaderType;
		std::string			ShaderModel;
		std::string			EntryFunction;
	};

	class K3D_API IShaderCompiler
	{
	public:
		virtual IShaderCompilerOutput*	Compile(ShaderCompilerOption const& option, const char * source) = 0;
		virtual ~IShaderCompiler() {}
	};
}

extern K3D_API ::k3d::IShaderCompiler * CreateShaderCompiler(::k3d::EShaderLang lang);

#endif