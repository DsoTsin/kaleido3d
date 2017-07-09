#pragma once

#include "Public/ShaderCompiler.h"

namespace k3d
{
    class MetalCompiler : public k3d::IShCompiler
    {
    public:
        MetalCompiler();
        ~MetalCompiler() override;
        NGFXShaderCompileResult	Compile(String const& src,
                                    NGFXShaderDesc const& inOp,
                                    NGFXShaderBundle & bundle) override;
        const char *        GetVersion();
    
    private:
        bool m_IsMac = true;
    };
}
