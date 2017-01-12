#pragma once
#ifndef __GLSLCompiler_h__
#define __GLSLCompiler_h__

#include "Public/ShaderCompiler.h"
#include <cassert>
#include <vector>

namespace k3d 
{
	typedef std::vector<uint32> SPIRV_T;

    struct GLSLangCompiler : public rhi::IShCompiler
    {
        typedef ::k3d::SharedPtr<IShCompiler> Ptr;
        virtual rhi::shc::EResult Compile(
                                          String const& src,
                                          rhi::ShaderDesc const& inOp,
                                          rhi::ShaderBundle & bundle) override;
		GLSLangCompiler();
        ~GLSLangCompiler() override;
    };
    
}

#endif
