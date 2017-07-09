#pragma once
#ifndef __GLSLCompiler_h__
#define __GLSLCompiler_h__

#include "Public/ShaderCompiler.h"
#include <cassert>
#include <vector>

namespace k3d 
{
	typedef std::vector<uint32> SPIRV_T;

    struct GLSLangCompiler : public k3d::IShCompiler
    {
        typedef ::k3d::SharedPtr<IShCompiler> Ptr;
        virtual NGFXShaderCompileResult Compile(
                                          String const& src,
                                          NGFXShaderDesc const& inOp,
                                          NGFXShaderBundle & bundle) override;
		GLSLangCompiler();
        ~GLSLangCompiler() override;
    };
    
}

#endif
