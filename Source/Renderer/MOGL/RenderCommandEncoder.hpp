#include "CommandEncoder.hpp"
#include <Config/Prerequisities.h>

enum class PrimitiveType : uint32_t {
	Points = 0x0000,
	Lines = 0x0001,
	LineLoop = 0x0002,
	LineStrip = 0x0003,
	Triangles = 0x0004,
	TriangleStrip = 0x0005,
	TriangleFan = 0x0006,
	Quads = 0x0007
};

class RenderCommandEncoder : public CommandEncoder {
public:
//    Setting Graphics Rendering State
//    setBlendColorRed:green:blue:alpha:
//    setCullMode:
//    setDepthBias:slopeScale:clamp:
//    setDepthStencilState:
//    setFrontFacingWinding:
//    setRenderPipelineState:
//    setScissorRect:
//    setStencilReferenceValue:
//	void setTriangleFillMode(PrimitiveType mode);
//    setViewport:
//    setVisibilityResultMode:offset:

//    Specifying Resources for a Vertex Function
//            setVertexBuffer:offset:atIndex:
//            setVertexBuffers:offsets:withRange:
//    setVertexSamplerState:atIndex:
//    setVertexSamplerStates:withRange:
//    setVertexSamplerState:lodMinClamp:lodMaxClamp:atIndex:
//    setVertexSamplerStates:lodMinClamps:lodMaxClamps:withRange:
//    setVertexTexture:atIndex:
//    setVertexTextures:withRange:

//    Specifying Resources for a Fragment Function
//            setFragmentBuffer:offset:atIndex:
//            setFragmentBuffers:offsets:withRange:
//    setFragmentSamplerState:atIndex:
//    setFragmentSamplerStates:withRange:
//    setFragmentSamplerState:lodMinClamp:lodMaxClamp:atIndex:
//    setFragmentSamplerStates:lodMinClamps:lodMaxClamps:withRange:
//    setFragmentTexture:atIndex:
//    setFragmentTextures:withRange:
//
//    Drawing Geometric Primitives
	void drawPrimitives(int vertexStart, int vertexCount, int instanceCount);
	void drawPrimitives(int vertexStart, int vertexCount);
//            drawIndexedPrimitives:indexCount:indexType:indexBuffer:indexBufferOffset:instanceCount:
//            drawIndexedPrimitives:indexCount:indexType:indexBuffer:indexBufferOffset:
//
//    Data Types
//    Primitive Type Codes
//            Index Type Codes
//    Visibility Result Modes
//            Culling Modes
//            Front-Facing Primitive Orientation Codes
//    Triangle Fill Modes

    void endEncoding() override;

private:

};