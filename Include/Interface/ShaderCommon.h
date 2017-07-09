#ifndef __ShaderCommon_h__
#define __ShaderCommon_h__

#include "../KTL/DynArray.hpp"
#include "../KTL/String.hpp"
#include "NGFX.h"

K3D_COMMON_NS
{
  KFORCE_INLINE::k3d::Archive& operator<<(::k3d::Archive& ar,
                                          NGFXShaderDesc const& attr)
  {
    ar << attr.Format << attr.Lang << attr.Profile << attr.Stage
       << attr.EntryFunction;
    return ar;
  }

  KFORCE_INLINE::k3d::Archive& operator>>(::k3d::Archive& ar, NGFXShaderDesc& attr)
  {
    ar >> attr.Format >> attr.Lang >> attr.Profile >> attr.Stage >>
      attr.EntryFunction;
    return ar;
  }

  KFORCE_INLINE::k3d::Archive& operator<<(::k3d::Archive& ar,
    NGFXShaderAttribute const& attr)
  {
    ar << attr.VarSemantic << attr.VarType << attr.VarLocation
      << attr.VarBindingPoint << attr.VarCount << attr.VarName;
    return ar;
  }

  KFORCE_INLINE::k3d::Archive& operator>>(::k3d::Archive& ar, NGFXShaderAttribute& attr)
  {
    ar >> attr.VarSemantic >> attr.VarType >> attr.VarLocation >>
      attr.VarBindingPoint >> attr.VarCount >> attr.VarName;
    return ar;
  }

  KFORCE_INLINE::k3d::Archive& operator<<(::k3d::Archive& ar,
    NGFXShaderUniform const& attr)
  {
    ar << attr.VarType << attr.VarOffset << attr.VarSzArray << attr.VarName;
    return ar;
  }

  KFORCE_INLINE::k3d::Archive& operator>>(::k3d::Archive& ar, NGFXShaderUniform& attr)
  {
    ar >> attr.VarType >> attr.VarOffset >> attr.VarSzArray >> attr.VarName;
    return ar;
  }

  KFORCE_INLINE bool operator==(NGFXShaderUniform const& lhs, NGFXShaderUniform const& rhs)
  {
    return rhs.VarType == lhs.VarType && rhs.VarOffset == lhs.VarOffset &&
      rhs.VarSzArray == lhs.VarSzArray && rhs.VarName == lhs.VarName;
  }

  KFORCE_INLINE::k3d::Archive& operator<<(::k3d::Archive& ar,
    NGFXShaderBinding const& attr)
  {
    ar << attr.VarType << attr.VarStage << attr.VarNumber << attr.VarName;
    return ar;
  }

  KFORCE_INLINE::k3d::Archive& operator>>(::k3d::Archive& ar, NGFXShaderBinding& attr)
  {
    ar >> attr.VarType >> attr.VarStage >> attr.VarNumber >> attr.VarName;
    return ar;
  }

  KFORCE_INLINE bool operator==(NGFXShaderBinding const& lhs, NGFXShaderBinding const& rhs)
  {
    return rhs.VarType == lhs.VarType && rhs.VarStage == lhs.VarStage &&
      rhs.VarNumber == lhs.VarNumber && rhs.VarName == lhs.VarName;
  }

  KFORCE_INLINE::k3d::Archive& operator<<(::k3d::Archive& ar,
    NGFXShaderBindingTable const& attr)
  {
    ar << attr.Bindings << attr.Uniforms;
    return ar;
  }

  KFORCE_INLINE::k3d::Archive& operator>>(::k3d::Archive& ar,
    NGFXShaderBindingTable& attr)
  {
    ar >> attr.Bindings >> attr.Uniforms;
    return ar;
  }

  KFORCE_INLINE NGFXShaderBindingTable operator|(NGFXShaderBindingTable const& a,
    NGFXShaderBindingTable const& b)
  {
#define MERGE_A_B(Member)                                                      \
  \
if(!a.Member.empty())                                                          \
  {                                                                            \
    \
result.Member.AddAll(a.Member);                                                \
    \
if(!b.Member.empty())                                                          \
    \
{                                                                       \
      \
for(auto ele                                                                   \
        : b.Member)                                                            \
      \
{                                                                     \
        \
if(!a.Member.Contains(ele))                                                    \
        \
{                                                                   \
          \
result.Member.Append(ele);                                                     \
        \
}                                                                   \
      \
}                                                                     \
    \
}                                                                       \
  \
}
    NGFXShaderBindingTable result = {};
    MERGE_A_B(Bindings)
      MERGE_A_B(Uniforms)
      MERGE_A_B(Sets)
#undef MERGE_A_B
      return result;
  }

  enum NGFXShaderCompileResult
  {
    NGFX_SHADER_COMPILE_OK,
    NGFX_SHADER_COMPILE_FAILED,
  };

  KFORCE_INLINE::k3d::Archive& operator<<(::k3d::Archive& ar,
                                          NGFXShaderBundle const& attr)
  {
    ar << attr.Desc << attr.BindingTable << attr.Attributes << attr.RawData;
    return ar;
  }

  KFORCE_INLINE::k3d::Archive& operator>>(::k3d::Archive& ar,
                                          NGFXShaderBundle& attr)
  {
    ar >> attr.Desc >> attr.BindingTable >> attr.Attributes >> attr.RawData;
    return ar;
  }
}

#endif