//
//  CommandContext.hpp
//  Renderer_Metal
//
//  Created by QinZhou on 15/9/2.
//
//

#ifndef __CommandContext_h__
#define __CommandContext_h__

#include "Common.h"

NS_K3D_METAL_BEGIN

class CommandContext {
public:
    CommandContext();
    
private:
    id<MTLCommandBuffer> m_CmdBuffer;
    id<MTLCommandQueue> m_CmdQueue;
};

class ComputeContext : public CommandContext {
public:
    ComputeContext();
    
private:
    id<MTLComputeCommandEncoder> m_ComputeEncoder;
};

class GraphicsContext : public CommandContext {
public:
    GraphicsContext();
    
    
private:
    id<MTLRenderCommandEncoder> m_RenderEncoder;
};


NS_K3D_METAL_END

#endif /* CommandContext_hpp */
