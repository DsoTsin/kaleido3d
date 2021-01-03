#pragma once

#include "common.h"

namespace MTL {
_MTL_ENUM(NS::UInteger, PixelFormat) {
    PixelFormatInvalid = 0,
    PixelFormatA8Unorm = 1,
    PixelFormatR8Unorm      = 10,
    PixelFormatR8Unorm_sRGB = 11,
    PixelFormatR8Snorm = 12,
    PixelFormatR8Uint = 13,
    PixelFormatR8Sint = 14,
    PixelFormatR16Unorm = 20,
    PixelFormatR16Snorm = 22,
    PixelFormatR16Uint = 23,
    PixelFormatR16Sint = 24,
    PixelFormatR16Float = 25,
    PixelFormatRG8Unorm = 30,
    PixelFormatRG8Unorm_sRGB = 31,
    PixelFormatRG8Snorm = 32,
    PixelFormatRG8Uint = 33,
    PixelFormatRG8Sint = 34,
    PixelFormatB5G6R5Unorm = 40,
    PixelFormatA1BGR5Unorm = 41,
    PixelFormatABGR4Unorm = 42,
    PixelFormatBGR5A1Unorm = 43,
    PixelFormatR32Uint = 53,
    PixelFormatR32Sint = 54,
    PixelFormatR32Float = 55,
    PixelFormatRG16Unorm = 60,
    PixelFormatRG16Snorm = 62,
    PixelFormatRG16Uint = 63,
    PixelFormatRG16Sint = 64,
    PixelFormatRG16Float = 65,
    PixelFormatRGBA8Unorm = 70,
    PixelFormatRGBA8Unorm_sRGB = 71,
    PixelFormatRGBA8Snorm = 72,
    PixelFormatRGBA8Uint = 73,
    PixelFormatRGBA8Sint = 74,
    PixelFormatBGRA8Unorm = 80,
    PixelFormatBGRA8Unorm_sRGB = 81,
    PixelFormatRGB10A2Unorm = 90,
    PixelFormatRGB10A2Uint = 91,
    PixelFormatRG11B10Float = 92,
    PixelFormatRGB9E5Float = 93,
    PixelFormatBGR10A2Unorm = 94,
    PixelFormatRG32Uint = 103,
    PixelFormatRG32Sint = 104,
    PixelFormatRG32Float = 105,
    PixelFormatRGBA16Unorm = 110,
    PixelFormatRGBA16Snorm = 112,
    PixelFormatRGBA16Uint = 113,
    PixelFormatRGBA16Sint = 114,
    PixelFormatRGBA16Float = 115,
    PixelFormatRGBA32Uint = 123,
    PixelFormatRGBA32Sint = 124,
    PixelFormatRGBA32Float = 125,
    PixelFormatBC1_RGBA = 130,
    PixelFormatBC1_RGBA_sRGB = 131,
    PixelFormatBC2_RGBA = 132,
    PixelFormatBC2_RGBA_sRGB = 133,
    PixelFormatBC3_RGBA = 134,
    PixelFormatBC3_RGBA_sRGB = 135,
    PixelFormatBC4_RUnorm = 140,
    PixelFormatBC4_RSnorm = 141,
    PixelFormatBC5_RGUnorm = 142,
    PixelFormatBC5_RGSnorm = 143,
    PixelFormatBC6H_RGBFloat = 150,
    PixelFormatBC6H_RGBUfloat = 151,
    PixelFormatBC7_RGBAUnorm = 152,
    PixelFormatBC7_RGBAUnorm_sRGB = 153,
    PixelFormatPVRTC_RGB_2BPP = 160,
    PixelFormatPVRTC_RGB_2BPP_sRGB = 161,
    PixelFormatPVRTC_RGB_4BPP = 162,
    PixelFormatPVRTC_RGB_4BPP_sRGB = 163,
    PixelFormatPVRTC_RGBA_2BPP = 164,
    PixelFormatPVRTC_RGBA_2BPP_sRGB = 165,
    PixelFormatPVRTC_RGBA_4BPP = 166,
    PixelFormatPVRTC_RGBA_4BPP_sRGB = 167,
    PixelFormatEAC_R11Unorm = 170,
    PixelFormatEAC_R11Snorm = 172,
    PixelFormatEAC_RG11Unorm = 174,
    PixelFormatEAC_RG11Snorm = 176,
    PixelFormatEAC_RGBA8 = 178,
    PixelFormatEAC_RGBA8_sRGB = 179,
    PixelFormatETC2_RGB8 = 180,
    PixelFormatETC2_RGB8_sRGB = 181,
    PixelFormatETC2_RGB8A1 = 182,
    PixelFormatETC2_RGB8A1_sRGB = 183,
    PixelFormatASTC_4x4_sRGB = 186,
    PixelFormatASTC_5x4_sRGB = 187,
    PixelFormatASTC_5x5_sRGB = 188,
    PixelFormatASTC_6x5_sRGB = 189,
    PixelFormatASTC_6x6_sRGB = 190,
    PixelFormatASTC_8x5_sRGB = 192,
    PixelFormatASTC_8x6_sRGB = 193,
    PixelFormatASTC_8x8_sRGB = 194,
    PixelFormatASTC_10x5_sRGB = 195,
    PixelFormatASTC_10x6_sRGB = 196,
    PixelFormatASTC_10x8_sRGB = 197,
    PixelFormatASTC_10x10_sRGB = 198,
    PixelFormatASTC_12x10_sRGB = 199,
    PixelFormatASTC_12x12_sRGB = 200,
    PixelFormatASTC_4x4_LDR = 204,
    PixelFormatASTC_5x4_LDR = 205,
    PixelFormatASTC_5x5_LDR = 206,
    PixelFormatASTC_6x5_LDR = 207,
    PixelFormatASTC_6x6_LDR = 208,
    PixelFormatASTC_8x5_LDR = 210,
    PixelFormatASTC_8x6_LDR = 211,
    PixelFormatASTC_8x8_LDR = 212,
    PixelFormatASTC_10x5_LDR = 213,
    PixelFormatASTC_10x6_LDR = 214,
    PixelFormatASTC_10x8_LDR = 215,
    PixelFormatASTC_10x10_LDR = 216,
    PixelFormatASTC_12x10_LDR = 217,
    PixelFormatASTC_12x12_LDR = 218,
    PixelFormatASTC_4x4_HDR = 222,
    PixelFormatASTC_5x4_HDR = 223,
    PixelFormatASTC_5x5_HDR = 224,
    PixelFormatASTC_6x5_HDR = 225,
    PixelFormatASTC_6x6_HDR = 226,
    PixelFormatASTC_8x5_HDR = 228,
    PixelFormatASTC_8x6_HDR = 229,
    PixelFormatASTC_8x8_HDR = 230,
    PixelFormatASTC_10x5_HDR = 231,
    PixelFormatASTC_10x6_HDR = 232,
    PixelFormatASTC_10x8_HDR = 233,
    PixelFormatASTC_10x10_HDR = 234,
    PixelFormatASTC_12x10_HDR = 235,
    PixelFormatASTC_12x12_HDR = 236,
    PixelFormatGBGR422 = 240,
    PixelFormatBGRG422 = 241,
    PixelFormatDepth16Unorm = 250,
    PixelFormatDepth32Float = 252,
    PixelFormatStencil8 = 253,
    PixelFormatDepth24Unorm_Stencil8 = 255,
    PixelFormatDepth32Float_Stencil8 = 260,
    PixelFormatX32_Stencil8 = 261,
    PixelFormatX24_Stencil8 = 262,
    PixelFormatBGRA10_XR = 552,
    PixelFormatBGRA10_XR_sRGB = 553,
    PixelFormatBGR10_XR = 554,
    PixelFormatBGR10_XR_sRGB = 555,
};

_MTL_ENUM(NS::UInteger, AttributeFormat) {
    AttributeFormatInvalid = 0,
    AttributeFormatUChar2 = 1,
    AttributeFormatUChar3 = 2,
    AttributeFormatUChar4 = 3,
    AttributeFormatChar2 = 4,
    AttributeFormatChar3 = 5,
    AttributeFormatChar4 = 6,
    AttributeFormatUChar2Normalized = 7,
    AttributeFormatUChar3Normalized = 8,
    AttributeFormatUChar4Normalized = 9,
    AttributeFormatChar2Normalized = 10,
    AttributeFormatChar3Normalized = 11,
    AttributeFormatChar4Normalized = 12,
    AttributeFormatUShort2 = 13,
    AttributeFormatUShort3 = 14,
    AttributeFormatUShort4 = 15,
    AttributeFormatShort2 = 16,
    AttributeFormatShort3 = 17,
    AttributeFormatShort4 = 18,
    AttributeFormatUShort2Normalized = 19,
    AttributeFormatUShort3Normalized = 20,
    AttributeFormatUShort4Normalized = 21,
    AttributeFormatShort2Normalized = 22,
    AttributeFormatShort3Normalized = 23,
    AttributeFormatShort4Normalized = 24,
    AttributeFormatHalf2 = 25,
    AttributeFormatHalf3 = 26,
    AttributeFormatHalf4 = 27,
    AttributeFormatFloat = 28,
    AttributeFormatFloat2 = 29,
    AttributeFormatFloat3 = 30,
    AttributeFormatFloat4 = 31,
    AttributeFormatInt = 32,
    AttributeFormatInt2 = 33,
    AttributeFormatInt3 = 34,
    AttributeFormatInt4 = 35,
    AttributeFormatUInt = 36,
    AttributeFormatUInt2 = 37,
    AttributeFormatUInt3 = 38,
    AttributeFormatUInt4 = 39,
    AttributeFormatInt1010102Normalized = 40,
    AttributeFormatUInt1010102Normalized = 41,
    AttributeFormatUChar4Normalized_BGRA = 42,
    AttributeFormatUChar = 45,
    AttributeFormatChar = 46,
    AttributeFormatUCharNormalized = 47,
    AttributeFormatCharNormalized = 48,
    AttributeFormatUShort = 49,
    AttributeFormatShort = 50,
    AttributeFormatUShortNormalized = 51,
    AttributeFormatShortNormalized = 52,
    AttributeFormatHalf = 53,
};

_MTL_ENUM(NS::UInteger, IndexType) {
    IndexTypeUInt16 = 0,
    IndexTypeUInt32 = 1,
};

_MTL_ENUM(NS::UInteger, StepFunction) {
    StepFunctionConstant = 0,
    StepFunctionPerVertex = 1,
    StepFunctionPerInstance = 2,
    StepFunctionPerPatch = 3,
    StepFunctionPerPatchControlPoint = 4,
    StepFunctionThreadPositionInGridX = 5,
    StepFunctionThreadPositionInGridY = 6,
    StepFunctionThreadPositionInGridXIndexed = 7,
    StepFunctionThreadPositionInGridYIndexed = 8,
};

_MTL_ENUM(NS::UInteger, VertexFormat) {
    VertexFormatInvalid = 0,
    VertexFormatUChar2 = 1,
    VertexFormatUChar3 = 2,
    VertexFormatUChar4 = 3,
    VertexFormatChar2 = 4,
    VertexFormatChar3 = 5,
    VertexFormatChar4 = 6,
    VertexFormatUChar2Normalized = 7,
    VertexFormatUChar3Normalized = 8,
    VertexFormatUChar4Normalized = 9,
    VertexFormatChar2Normalized = 10,
    VertexFormatChar3Normalized = 11,
    VertexFormatChar4Normalized = 12,
    VertexFormatUShort2 = 13,
    VertexFormatUShort3 = 14,
    VertexFormatUShort4 = 15,
    VertexFormatShort2 = 16,
    VertexFormatShort3 = 17,
    VertexFormatShort4 = 18,
    VertexFormatUShort2Normalized = 19,
    VertexFormatUShort3Normalized = 20,
    VertexFormatUShort4Normalized = 21,
    VertexFormatShort2Normalized = 22,
    VertexFormatShort3Normalized = 23,
    VertexFormatShort4Normalized = 24,
    VertexFormatHalf2 = 25,
    VertexFormatHalf3 = 26,
    VertexFormatHalf4 = 27,
    VertexFormatFloat = 28,
    VertexFormatFloat2 = 29,
    VertexFormatFloat3 = 30,
    VertexFormatFloat4 = 31,
    VertexFormatInt = 32,
    VertexFormatInt2 = 33,
    VertexFormatInt3 = 34,
    VertexFormatInt4 = 35,
    VertexFormatUInt = 36,
    VertexFormatUInt2 = 37,
    VertexFormatUInt3 = 38,
    VertexFormatUInt4 = 39,
    VertexFormatInt1010102Normalized = 40,
    VertexFormatUInt1010102Normalized = 41,
    VertexFormatUChar4Normalized_BGRA = 42,
    VertexFormatUChar = 45,
    VertexFormatChar = 46,
    VertexFormatUCharNormalized = 47,
    VertexFormatCharNormalized = 48,
    VertexFormatUShort = 49,
    VertexFormatShort = 50,
    VertexFormatUShortNormalized = 51,
    VertexFormatShortNormalized = 52,
    VertexFormatHalf = 53,
};

_MTL_ENUM(NS::UInteger, VertexStepFunction) {
    VertexStepFunctionConstant = 0,
    VertexStepFunctionPerVertex = 1,
    VertexStepFunctionPerInstance = 2,
    VertexStepFunctionPerPatch = 3,
    VertexStepFunctionPerPatchControlPoint = 4,
};

_MTL_ENUM(NS::UInteger, DataType) {
    DataTypeNone = 0,
    DataTypeStruct = 1,
    DataTypeArray = 2,
    DataTypeFloat = 3,
    DataTypeFloat2 = 4,
    DataTypeFloat3 = 5,
    DataTypeFloat4 = 6,
    DataTypeFloat2x2 = 7,
    DataTypeFloat2x3 = 8,
    DataTypeFloat2x4 = 9,
    DataTypeFloat3x2 = 10,
    DataTypeFloat3x3 = 11,
    DataTypeFloat3x4 = 12,
    DataTypeFloat4x2 = 13,
    DataTypeFloat4x3 = 14,
    DataTypeFloat4x4 = 15,
    DataTypeHalf = 16,
    DataTypeHalf2 = 17,
    DataTypeHalf3 = 18,
    DataTypeHalf4 = 19,
    DataTypeHalf2x2 = 20,
    DataTypeHalf2x3 = 21,
    DataTypeHalf2x4 = 22,
    DataTypeHalf3x2 = 23,
    DataTypeHalf3x3 = 24,
    DataTypeHalf3x4 = 25,
    DataTypeHalf4x2 = 26,
    DataTypeHalf4x3 = 27,
    DataTypeHalf4x4 = 28,
    DataTypeInt = 29,
    DataTypeInt2 = 30,
    DataTypeInt3 = 31,
    DataTypeInt4 = 32,
    DataTypeUInt = 33,
    DataTypeUInt2 = 34,
    DataTypeUInt3 = 35,
    DataTypeUInt4 = 36,
    DataTypeShort = 37,
    DataTypeShort2 = 38,
    DataTypeShort3 = 39,
    DataTypeShort4 = 40,
    DataTypeUShort = 41,
    DataTypeUShort2 = 42,
    DataTypeUShort3 = 43,
    DataTypeUShort4 = 44,
    DataTypeChar = 45,
    DataTypeChar2 = 46,
    DataTypeChar3 = 47,
    DataTypeChar4 = 48,
    DataTypeUChar = 49,
    DataTypeUChar2 = 50,
    DataTypeUChar3 = 51,
    DataTypeUChar4 = 52,
    DataTypeBool = 53,
    DataTypeBool2 = 54,
    DataTypeBool3 = 55,
    DataTypeBool4 = 56,
    DataTypeTexture = 58,
    DataTypeSampler = 59,
    DataTypePointer = 60,
    DataTypeR8Unorm = 62,
    DataTypeR8Snorm = 63,
    DataTypeR16Unorm = 64,
    DataTypeR16Snorm = 65,
    DataTypeRG8Unorm = 66,
    DataTypeRG8Snorm = 67,
    DataTypeRG16Unorm = 68,
    DataTypeRG16Snorm = 69,
    DataTypeRGBA8Unorm = 70,
    DataTypeRGBA8Unorm_sRGB = 71,
    DataTypeRGBA8Snorm = 72,
    DataTypeRGBA16Unorm = 73,
    DataTypeRGBA16Snorm = 74,
    DataTypeRGB10A2Unorm = 75,
    DataTypeRG11B10Float = 76,
    DataTypeRGB9E5Float = 77,
    DataTypeRenderPipeline = 78,
    DataTypeComputePipeline = 79,
    DataTypeIndirectCommandBuffer = 80,
    DataTypeLong = 81,
    DataTypeLong2 = 82,
    DataTypeLong3 = 83,
    DataTypeLong4 = 84,
    DataTypeULong = 85,
    DataTypeULong2 = 86,
    DataTypeULong3 = 87,
    DataTypeULong4 = 88,
    DataTypeVisibleFunctionTable = 115,
    DataTypeIntersectionFunctionTable = 116,
    DataTypePrimitiveAccelerationStructure = 117,
    DataTypeInstanceAccelerationStructure = 118,
};

_MTL_ENUM(NS::UInteger, ArgumentType) {
    ArgumentTypeBuffer = 0,
    ArgumentTypeThreadgroupMemory = 1,
    ArgumentTypeTexture = 2,
    ArgumentTypeSampler = 3,
    ArgumentTypeImageblockData = 16,
    ArgumentTypeImageblock = 17,
    ArgumentTypeVisibleFunctionTable = 24,
    ArgumentTypePrimitiveAccelerationStructure = 25,
    ArgumentTypeInstanceAccelerationStructure = 26,
    ArgumentTypeIntersectionFunctionTable = 27,
};

_MTL_ENUM(NS::UInteger, ArgumentAccess) {
    ArgumentAccessReadOnly = 0,
    ArgumentAccessReadWrite = 1,
    ArgumentAccessWriteOnly = 2,
};

_MTL_ENUM(NS::UInteger, CommandBufferStatus) {
    CommandBufferStatusNotEnqueued = 0,
    CommandBufferStatusEnqueued = 1,
    CommandBufferStatusCommitted = 2,
    CommandBufferStatusScheduled = 3,
    CommandBufferStatusCompleted = 4,
    CommandBufferStatusError = 5,
};

_MTL_ENUM(NS::UInteger, CommandBufferError) {
    CommandBufferErrorNone = 0,
    CommandBufferErrorTimeout = 2,
    CommandBufferErrorPageFault = 3,
    CommandBufferErrorAccessRevoked = 4,
    CommandBufferErrorBlacklisted = 4,
    CommandBufferErrorNotPermitted = 7,
    CommandBufferErrorOutOfMemory = 8,
    CommandBufferErrorInvalidResource = 9,
    CommandBufferErrorMemoryless = 10,
    CommandBufferErrorDeviceRemoved = 11,
    CommandBufferErrorStackOverflow = 12,
};

_MTL_OPTIONS(NS::UInteger, CommandBufferErrorOption) {
    CommandBufferErrorOptionNone = 0,
    CommandBufferErrorOptionEncoderExecutionStatus = 1,
};

_MTL_ENUM(NS::Integer, CommandEncoderErrorState) {
    CommandEncoderErrorStateUnknown = 0,
    CommandEncoderErrorStateCompleted = 1,
    CommandEncoderErrorStateAffected = 2,
    CommandEncoderErrorStatePending = 3,
    CommandEncoderErrorStateFaulted = 4,
};

_MTL_ENUM(NS::UInteger, CompareFunction) {
    CompareFunctionNever = 0,
    CompareFunctionLess = 1,
    CompareFunctionEqual = 2,
    CompareFunctionLessEqual = 3,
    CompareFunctionGreater = 4,
    CompareFunctionNotEqual = 5,
    CompareFunctionGreaterEqual = 6,
    CompareFunctionAlways = 7,
};

_MTL_ENUM(NS::UInteger, StencilOperation) {
    StencilOperationKeep = 0,
    StencilOperationZero = 1,
    StencilOperationReplace = 2,
    StencilOperationIncrementClamp = 3,
    StencilOperationDecrementClamp = 4,
    StencilOperationInvert = 5,
    StencilOperationIncrementWrap = 6,
    StencilOperationDecrementWrap = 7,
};

_MTL_ENUM(NS::UInteger, PrimitiveType) {
    PrimitiveTypePoint = 0,
    PrimitiveTypeLine = 1,
    PrimitiveTypeLineStrip = 2,
    PrimitiveTypeTriangle = 3,
    PrimitiveTypeTriangleStrip = 4,
};

_MTL_ENUM(NS::UInteger, VisibilityResultMode) {
    VisibilityResultModeDisabled = 0,
    VisibilityResultModeBoolean = 1,
    VisibilityResultModeCounting = 2,
};

_MTL_ENUM(NS::UInteger, CullMode) {
    CullModeNone = 0,
    CullModeFront = 1,
    CullModeBack = 2,
};

_MTL_ENUM(NS::UInteger, Winding) {
    WindingClockwise = 0,
    WindingCounterClockwise = 1,
};

_MTL_ENUM(NS::UInteger, DepthClipMode) {
    DepthClipModeClip = 0,
    DepthClipModeClamp = 1,
};

_MTL_ENUM(NS::UInteger, TriangleFillMode) {
    TriangleFillModeFill = 0,
    TriangleFillModeLines = 1,
};

_MTL_ENUM(NS::UInteger, LoadAction) {
    LoadActionDontCare = 0,
    LoadActionLoad = 1,
    LoadActionClear = 2,
};

_MTL_ENUM(NS::UInteger, StoreAction) {
    StoreActionDontCare = 0,
    StoreActionStore = 1,
    StoreActionMultisampleResolve = 2,
    StoreActionStoreAndMultisampleResolve = 3,
    StoreActionUnknown = 4,
    StoreActionCustomSampleDepthStore = 5,
};

_MTL_OPTIONS(NS::UInteger, StoreActionOptions) {
    StoreActionOptionNone = 0,
    StoreActionOptionValidMask = 1,
    StoreActionOptionCustomSamplePositions = 1,
};

_MTL_ENUM(NS::UInteger, MultisampleStencilResolveFilter) {
    MultisampleStencilResolveFilterSample0 = 0,
    MultisampleStencilResolveFilterDepthResolvedSample = 1,
};

_MTL_ENUM(NS::UInteger, BlendFactor) {
    BlendFactorZero = 0,
    BlendFactorOne = 1,
    BlendFactorSourceColor = 2,
    BlendFactorOneMinusSourceColor = 3,
    BlendFactorSourceAlpha = 4,
    BlendFactorOneMinusSourceAlpha = 5,
    BlendFactorDestinationColor = 6,
    BlendFactorOneMinusDestinationColor = 7,
    BlendFactorDestinationAlpha = 8,
    BlendFactorOneMinusDestinationAlpha = 9,
    BlendFactorSourceAlphaSaturated = 10,
    BlendFactorBlendColor = 11,
    BlendFactorOneMinusBlendColor = 12,
    BlendFactorBlendAlpha = 13,
    BlendFactorOneMinusBlendAlpha = 14,
    BlendFactorSource1Color = 15,
    BlendFactorOneMinusSource1Color = 16,
    BlendFactorSource1Alpha = 17,
    BlendFactorOneMinusSource1Alpha = 18,
};

_MTL_ENUM(NS::UInteger, BlendOperation) {
    BlendOperationAdd = 0,
    BlendOperationSubtract = 1,
    BlendOperationReverseSubtract = 2,
    BlendOperationMin = 3,
    BlendOperationMax = 4,
};

_MTL_OPTIONS(NS::UInteger, ColorWriteMask) {
    ColorWriteMaskNone = 0,
    ColorWriteMaskAlpha = 1,
    ColorWriteMaskBlue = 2,
    ColorWriteMaskGreen = 4,
    ColorWriteMaskRed = 8,
    ColorWriteMaskAll = 15,
};

_MTL_ENUM(NS::UInteger, PrimitiveTopologyClass) {
    PrimitiveTopologyClassUnspecified = 0,
    PrimitiveTopologyClassPoint = 1,
    PrimitiveTopologyClassLine = 2,
    PrimitiveTopologyClassTriangle = 3,
};

_MTL_ENUM(NS::UInteger, TessellationPartitionMode) {
    TessellationPartitionModePow2 = 0,
    TessellationPartitionModeInteger = 1,
    TessellationPartitionModeFractionalOdd = 2,
    TessellationPartitionModeFractionalEven = 3,
};

_MTL_ENUM(NS::UInteger, TessellationFactorStepFunction) {
    TessellationFactorStepFunctionConstant = 0,
    TessellationFactorStepFunctionPerPatch = 1,
    TessellationFactorStepFunctionPerInstance = 2,
    TessellationFactorStepFunctionPerPatchAndPerInstance = 3,
};

_MTL_ENUM(NS::UInteger, TessellationFactorFormat) {
    TessellationFactorFormatHalf = 0,
};

_MTL_ENUM(NS::UInteger, TessellationControlPointIndexType) {
    TessellationControlPointIndexTypeNone = 0,
    TessellationControlPointIndexTypeUInt16 = 1,
    TessellationControlPointIndexTypeUInt32 = 2,
};

_MTL_ENUM(NS::UInteger, PurgeableState) {
    PurgeableStateKeepCurrent = 1,
    PurgeableStateNonVolatile = 2,
    PurgeableStateVolatile = 3,
    PurgeableStateEmpty = 4,
};

_MTL_ENUM(NS::UInteger, CPUCacheMode) {
    CPUCacheModeDefaultCache = 0,
    CPUCacheModeWriteCombined = 1,
};

_MTL_ENUM(NS::UInteger, StorageMode) {
    StorageModeShared = 0,
    StorageModeManaged = 1,
    StorageModePrivate = 2,
    StorageModeMemoryless = 3,
};

_MTL_ENUM(NS::UInteger, HazardTrackingMode) {
    HazardTrackingModeDefault = 0,
    HazardTrackingModeUntracked = 1,
    HazardTrackingModeTracked = 2,
};

_MTL_OPTIONS(NS::UInteger, ResourceOptions) {
    ResourceStorageModeShared = 0,
    ResourceHazardTrackingModeDefault = 0,
    ResourceCPUCacheModeDefaultCache = 0,
    ResourceOptionCPUCacheModeDefault = 0,
    ResourceCPUCacheModeWriteCombined = 1,
    ResourceOptionCPUCacheModeWriteCombined = 1,
    ResourceStorageModeManaged = 16,
    ResourceStorageModePrivate = 32,
    ResourceStorageModeMemoryless = 48,
    ResourceHazardTrackingModeUntracked = 256,
    ResourceHazardTrackingModeTracked = 512,
};

_MTL_ENUM(NS::UInteger, SparseTextureMappingMode) {
    SparseTextureMappingModeMap = 0,
    SparseTextureMappingModeUnmap = 1,
};

_MTL_ENUM(NS::UInteger, SamplerMinMagFilter) {
    SamplerMinMagFilterNearest = 0,
    SamplerMinMagFilterLinear = 1,
};

_MTL_ENUM(NS::UInteger, SamplerMipFilter) {
    SamplerMipFilterNotMipmapped = 0,
    SamplerMipFilterNearest = 1,
    SamplerMipFilterLinear = 2,
};

_MTL_ENUM(NS::UInteger, SamplerAddressMode) {
    SamplerAddressModeClampToEdge = 0,
    SamplerAddressModeMirrorClampToEdge = 1,
    SamplerAddressModeRepeat = 2,
    SamplerAddressModeMirrorRepeat = 3,
    SamplerAddressModeClampToZero = 4,
    SamplerAddressModeClampToBorderColor = 5,
};

_MTL_ENUM(NS::UInteger, SamplerBorderColor) {
    SamplerBorderColorTransparentBlack = 0,
    SamplerBorderColorOpaqueBlack = 1,
    SamplerBorderColorOpaqueWhite = 2,
};

_MTL_ENUM(NS::UInteger, TextureType) {
    TextureType1D = 0,
    TextureType1DArray = 1,
    TextureType2D = 2,
    TextureType2DArray = 3,
    TextureType2DMultisample = 4,
    TextureTypeCube = 5,
    TextureTypeCubeArray = 6,
    TextureType3D = 7,
    TextureType2DMultisampleArray = 8,
    TextureTypeTextureBuffer = 9,
};

_MTL_ENUM(uint8_t, TextureSwizzle) {
    TextureSwizzleZero = 0,
    TextureSwizzleOne = 1,
    TextureSwizzleRed = 2,
    TextureSwizzleGreen = 3,
    TextureSwizzleBlue = 4,
    TextureSwizzleAlpha = 5,
};

_MTL_ENUM(NS::Integer, TextureCompressionType) {
    TextureCompressionTypeLossless = 0,
    TextureCompressionTypeLossy = 1,
};

_MTL_ENUM(NS::Integer, HeapType) {
    HeapTypeAutomatic = 0,
    HeapTypePlacement = 1,
    HeapTypeSparse = 2,
};

}