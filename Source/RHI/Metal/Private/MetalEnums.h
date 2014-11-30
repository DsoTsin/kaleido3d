#pragma once
#ifndef __MetalEnums_h__
#define __MetalEnums_h__

#include "../Common.h"
#include <IRHIDefs.h>


NS_K3D_METAL_BEGIN
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

NS_K3D_METAL_END

#endif