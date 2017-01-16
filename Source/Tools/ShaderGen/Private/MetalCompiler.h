#pragma once

#include "Public/ShaderCompiler.h"

namespace k3d
{
    class MetalCompiler : public rhi::IShCompiler
    {
    public:
        MetalCompiler();
        ~MetalCompiler() override;
        rhi::shc::EResult	Compile(String const& src,
                                    rhi::ShaderDesc const& inOp,
                                    rhi::ShaderBundle & bundle) override;
        const char *        GetVersion();
    
    private:
        bool m_IsMac = true;
    };
}
