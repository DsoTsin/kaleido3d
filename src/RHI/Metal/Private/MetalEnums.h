#pragma once
#ifndef __MetalEnums_h__
#define __MetalEnums_h__

#include "../Common.h"
#include <Interface/IRHIDefs.h>


NS_K3D_METAL_BEGIN

namespace
{
    /**
     EPF_RGBA16Uint,
     EPF_RGBA32Float,
     EPF_RGBA8Unorm,
     EPF_RGBA8Unorm_sRGB,
     EPF_R11G11B10Float,
     EPF_D32Float,
     EPF_RGB32Float,
     EPF_RGB8Unorm,
     */
    MTLPixelFormat g_PixelFormat[] = {
        MTLPixelFormatRGBA16Uint,
        MTLPixelFormatRGBA32Float,
        MTLPixelFormatRGBA8Unorm,
        MTLPixelFormatRGBA8Unorm_sRGB,
        MTLPixelFormatRG11B10Float,
        MTLPixelFormatDepth32Float,
        MTLPixelFormatInvalid,
        MTLPixelFormatInvalid,
        MTLPixelFormatBGRA8Unorm,
        MTLPixelFormatBGRA8Unorm_sRGB,
        MTLPixelFormatRGBA16Float,
    };
    
    /*
     Points,
     Lines,
     Triangles,
     TriangleStrip,
     */
    MTLPrimitiveType g_PrimitiveType[] = {
        MTLPrimitiveTypePoint,
        MTLPrimitiveTypeLine,
        MTLPrimitiveTypeTriangle,
        MTLPrimitiveTypeTriangleStrip,
    };
    /*
     None,
     Front,
     Back,
     */
    MTLCullMode g_CullMode[] = {
        MTLCullModeNone,
        MTLCullModeFront,
        MTLCullModeBack
    };
    /*
     WireFrame,
     Solid,
     */
    MTLTriangleFillMode g_FillMode[] = {
        MTLTriangleFillModeLines,
        MTLTriangleFillModeFill
    };
    /*
     Add,
     Sub,
     */
    MTLBlendOperation g_BlendOperation[] = {
        MTLBlendOperationAdd,
        MTLBlendOperationSubtract
    };
    /*
     Zero,
     One,
     SrcColor,
     DestColor,
     SrcAlpha,
     DestAlpha,
     BlendTypeNum
     */
    MTLBlendFactor g_BlendFactor[] = {
        MTLBlendFactorZero,
        MTLBlendFactorOne,
        MTLBlendFactorSourceColor,
        MTLBlendFactorDestinationColor,
        MTLBlendFactorSourceAlpha,
        MTLBlendFactorDestinationAlpha
    };
    /*
     Keep,
     Zero,
     Replace,
     Invert,
     Increment,
     Decrement,
     */
    MTLStencilOperation g_StencilOp[] = {
        MTLStencilOperationKeep,
        MTLStencilOperationZero,
        MTLStencilOperationReplace,
        MTLStencilOperationInvert,
        MTLStencilOperationIncrementClamp, // TODO
        MTLStencilOperationDecrementClamp // TODO
    };
    /*Never,
     Less,
     Equal,
     LessEqual,
     Greater,
     NotEqual,
     GreaterEqual,
     Always,
     */
    MTLCompareFunction g_ComparisonFunc[] = {
        MTLCompareFunctionLess,
        MTLCompareFunctionEqual,
        MTLCompareFunctionLessEqual,
        MTLCompareFunctionGreater,
        MTLCompareFunctionNotEqual,
        MTLCompareFunctionGreaterEqual,
        MTLCompareFunctionAlways
    };
    /*
     WriteZero,
     WriteAll,
     */
    
    /*enum EVertexFormat
     {
     EVF_Float1x32,
     EVF_Float2x32,
     EVF_Float3x32,
     EVF_Float4x32,
     */
    MTLVertexFormat g_VertexFormats[] = {
        MTLVertexFormatFloat,
        MTLVertexFormatFloat2,
        MTLVertexFormatFloat3,
        MTLVertexFormatFloat4,
    };
    
    MTLVertexStepFunction g_VertexInputRates[] = {
        MTLVertexStepFunctionPerVertex,
        MTLVertexStepFunctionPerInstance,
    };
}

NS_K3D_METAL_END

#endif
