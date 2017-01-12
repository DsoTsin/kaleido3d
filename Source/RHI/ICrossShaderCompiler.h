#pragma once 

#include "IRHIDefs.h"
#include <Core/Module.h>

namespace rhi
{
    enum EShaderFormat
    {
        EShFmt_Text,
        EShFmt_ByteCode,
        ENumShFmt
    };
    
    enum EShaderLang
    {
        EShLang_GLSL,
        EShLang_ESSL,
        EShLang_VkGLSL,
        EShLang_HLSL,
        EShLang_MetalSL,
    };
    
    enum EShaderProfile
    {
        EShProfile_Legacy,  // SM4_x, ES2.0,
        EShProfile_Modern,  // SM5_x, ES3.1, GLSL4XX, MSL
    };
    
	/**
	 * Format. Lang. Profile. Stage. Entry
	 */
    struct ShaderDesc
    {
        EShaderFormat   Format;
        EShaderLang     Lang;
        EShaderProfile  Profile;
        EShaderType     Stage;
        String          EntryFunction;
    };
    
    KFORCE_INLINE ::k3d::Archive & operator << (::k3d::Archive & ar, ShaderDesc const & attr)
    {
        ar << attr.Format << attr.Lang << attr.Profile << attr.Stage << attr.EntryFunction;
        return ar;
    }
    
    KFORCE_INLINE ::k3d::Archive & operator >> (::k3d::Archive & ar, ShaderDesc & attr)
    {
        ar >> attr.Format >> attr.Lang >> attr.Profile >> attr.Stage >> attr.EntryFunction;
        return ar;
    }
    
    namespace shc
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
            String		VarName;
            ESemantic	VarSemantic;
            EDataType	VarType;
            uint32		VarLocation;
            uint32		VarBindingPoint;
            uint32		VarCount;
            
            Attribute(const String& name, ESemantic semantic, EDataType dataType, uint32 location, uint32 bindingPoint, uint32 varCount)
            : VarName(name), VarSemantic(semantic), VarType(dataType), VarLocation(location), VarBindingPoint(bindingPoint), VarCount(varCount)
            {}
        };
        
        typedef ::k3d::DynArray<Attribute> Attributes;
        
        KFORCE_INLINE ::k3d::Archive & operator << (::k3d::Archive & ar, Attribute const & attr)
        {
            ar << attr.VarSemantic << attr.VarType << attr.VarLocation << attr.VarBindingPoint << attr.VarCount << attr.VarName;
            return ar;
        }
        
        KFORCE_INLINE ::k3d::Archive & operator >> (::k3d::Archive & ar, Attribute & attr)
        {
            ar >> attr.VarSemantic >> attr.VarType >> attr.VarLocation >> attr.VarBindingPoint >> attr.VarCount >> attr.VarName;
            return ar;
        }
        
        struct Uniform
        {
            Uniform() K3D_NOEXCEPT : VarType(EDataType::EUnknown), VarName(""), VarOffset(0), VarSzArray(0) {}
            
            Uniform(EDataType type, String const & name, uint32 offset, uint32 szArray=0)
            : VarType(type), VarName(name), VarOffset(offset), VarSzArray(szArray) {}
            
            virtual ~Uniform() {}
            
            EDataType	VarType;
            String 		VarName;
            uint32		VarOffset;
            uint32		VarSzArray;
        };
        
        KFORCE_INLINE ::k3d::Archive & operator << (::k3d::Archive & ar, Uniform const & attr)
        {
            ar << attr.VarType << attr.VarOffset << attr.VarSzArray << attr.VarName;
            return ar;
        }
        
        KFORCE_INLINE ::k3d::Archive & operator >> (::k3d::Archive & ar, Uniform & attr)
        {
            ar >> attr.VarType >> attr.VarOffset >> attr.VarSzArray >> attr.VarName;
            return ar;
        }
        
        KFORCE_INLINE bool operator==(Uniform const &lhs, Uniform const &rhs)
        {
            return rhs.VarType == lhs.VarType && rhs.VarOffset == lhs.VarOffset && rhs.VarSzArray == lhs.VarSzArray &&
            rhs.VarName == lhs.VarName;
        }
        
        class Constant : public Uniform
        {
        public:
            
            Constant() {}
            ~Constant() override {}
            
        };
        
        struct Binding
        {
            Binding() K3D_NOEXCEPT : VarType(EBindType::EUndefined), VarName(""), VarStage(rhi::ES_Vertex), VarNumber(0) {}
            
            Binding(EBindType t, std::string n, rhi::EShaderType st, uint32 num)
            : VarType(t), VarName(n.c_str()), VarStage(st), VarNumber(num){}
            
            EBindType			VarType;
            String				VarName;
            EShaderType         VarStage;
            uint32				VarNumber;
        };
        
        KFORCE_INLINE ::k3d::Archive & operator << (::k3d::Archive & ar, Binding const & attr)
        {
            ar << attr.VarType << attr.VarStage << attr.VarNumber << attr.VarName;
            return ar;
        }
        
        KFORCE_INLINE ::k3d::Archive & operator >> (::k3d::Archive & ar, Binding & attr)
        {
            ar >> attr.VarType >> attr.VarStage >> attr.VarNumber >> attr.VarName;
            return ar;
        }
        
        KFORCE_INLINE bool operator==(Binding const &lhs, Binding const &rhs)
        {
            return rhs.VarType == lhs.VarType && rhs.VarStage == lhs.VarStage && rhs.VarNumber == lhs.VarNumber &&
            rhs.VarName == lhs.VarName;
        }
        
        struct Set
        {
            typedef uint32 VarIndex;
            /*bool operator==(Set const &rhs) 
             {
             return ...
             }*/
        };
        
        struct BindingTable
        {
            ::k3d::DynArray<Binding>		Bindings;
            ::k3d::DynArray<Uniform>		Uniforms;
            ::k3d::DynArray<Set::VarIndex>	Sets;
            
            BindingTable() K3D_NOEXCEPT = default;
            
            BindingTable(BindingTable const &rhs)
            {
                Bindings = rhs.Bindings;
                Uniforms = rhs.Uniforms;
                Sets = rhs.Sets;
            }
            
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
        
        KFORCE_INLINE ::k3d::Archive & operator << (::k3d::Archive & ar, BindingTable const & attr)
        {
            ar << attr.Bindings << attr.Uniforms;
            return ar;
        }
        
        KFORCE_INLINE ::k3d::Archive & operator >> (::k3d::Archive & ar, BindingTable & attr)
        {
            ar >> attr.Bindings >> attr.Uniforms;
            return ar;
        }
        
        KFORCE_INLINE BindingTable operator | (BindingTable const & a, BindingTable const & b) {
#define MERGE_A_B(Member) \
if (!a.Member.empty()) {\
result.Member.AddAll(a.Member);\
if (!b.Member.empty())\
{\
for (auto ele : b.Member)\
{\
if (!a.Member.Contains(ele))\
{\
result.Member.Append(ele);\
}\
}\
}\
}
            BindingTable result = {};
            MERGE_A_B(Bindings)
            MERGE_A_B(Uniforms)
            MERGE_A_B(Sets)
#undef MERGE_A_B
            return result;
        }
        
        enum EResult
        {
            E_Ok,
            E_Failed,
        };
    }
    
    struct ShaderBundle
    {
        ShaderDesc        Desc;
        shc::BindingTable BindingTable;
        shc::Attributes   Attributes;
        String            RawData;
    };
    
    KFORCE_INLINE ::k3d::Archive & operator << (::k3d::Archive & ar, ShaderBundle const & attr)
    {
        ar << attr.Desc << attr.BindingTable << attr.Attributes << attr.RawData;
        return ar;
    }
    
    KFORCE_INLINE ::k3d::Archive & operator >> (::k3d::Archive & ar, ShaderBundle & attr)
    {
        ar >> attr.Desc >> attr.BindingTable >> attr.Attributes >> attr.RawData;
        return ar;
    }
    
    /**
     * ShaderCompiler interface
     */
    struct IShCompiler
    {
        typedef ::k3d::SharedPtr<IShCompiler> Ptr;
        virtual shc::EResult Compile(String const& src, ShaderDesc const& inOp, ShaderBundle & bundle) = 0;
        virtual ~IShCompiler() {}
    };
    
    class IShModule : public ::k3d::IModule
    {
    public:
        virtual ~IShModule() {}
        virtual IShCompiler::Ptr CreateShaderCompiler(ERHIType const&) = 0;
		//virtual ::k3d::DynArray<String> ListAvailableShaderLanguage() const = 0;
    };
}
