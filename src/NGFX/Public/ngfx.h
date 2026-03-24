#ifndef __ngfx_h__
#define __ngfx_h__
#include "ngfx_atomic.h"
#include "ngfx_ptr.h"
#include "ngfx_allocator.h"
#include "ngfx_container.h"
namespace ngfx {
enum class Result : uint8_t {
  Ok,
  Failed,
  ParamError,
  DeviceNotFound,
};
enum class Backend : uint8_t {
  Vulkan,
  Metal,
  Direct3D12,
  NVN,
  Agc,
};
enum class DeviceType : uint8_t {
  Integrate,
  Discrete,
  Virtual,
};
enum class HardwareTier : uint8_t {
  Software,
  Graphics,
  Compute,
  Raytracing,
};
enum class PipelineType : uint8_t {
  Graphics,
  Compute,
  RayTracing,
};
enum class ColorSpace : uint8_t {
  SRGBNonLinear,
};
enum class PixelFormat : uint8_t {
  Invalid,
  R8Unorm,
  R8Unorm_sRGB,
  R8Snorm,
  R8Uint,
  R8Sint,
  R16Unorm,
  R16Snorm,
  R16Uint,
  R16Sint,
  R16Float,
  RG8Unorm,
  RG8Unorm_sRGB,
  RG8Snorm,
  RG8Uint,
  RG8Sint,
  R32Uint,
  R32Sint,
  R32Float,
  RG16Unorm,
  RG16Snorm,
  RG16Uint,
  RG16Sint,
  RG16Float,
  RGBA8Unorm,
  RGBA8Unorm_sRGB,
  RGBA8Snorm,
  RGBA8Uint,
  RGBA8Sint,
  BGRA8Unorm,
  BGRA8Unorm_sRGB,
  RG11B10Float,
  RGB9E5Float,
  RG32Uint,
  RG32Sint,
  RG32Float,
  RGBA16Unorm,
  RGBA16Snorm,
  RGBA16Uint,
  RGBA16Sint,
  RGBA16Float,
  RGBA32Uint,
  RGBA32Sint,
  RGBA32Float,
  Depth16Unorm,
  Depth32Float,
  Stencil8,
  Depth24UnormStencil8,
  Depth32FloatStencil8,
  RGB8Unorm_ETC2,
  RGB8_sRGB_ETC2,
  RGBA8Unorm_ETC2,
  RGBA8_sRGB_ETC2,
  Block2BPPUnorm_PVRTC1,
  Block4BPPUnorm_PVRTC1,
  Block2BPPUnorm_PVRTC2,
  Block4BPPUnorm_PVRTC2,
  Block2BPP_sRGB_PVRTC1,
  Block4BPP_sRGB_PVRTC1,
  Block2BPP_sRGB_PVRTC2,
  Block4BPP_sRGB_PVRTC2,
};
enum class VertexFormat : uint8_t {
  Invalid,
  UChar,
  UChar2,
  UChar3,
  UChar4,
  Char,
  Char2,
  Char3,
  Char4,
  UShort,
  UShort2,
  UShort3,
  UShort4,
  Short,
  Short2,
  Short3,
  Short4,
  Half,
  Half2,
  Half3,
  Half4,
  Float,
  Float2,
  Float3,
  Float4,
  UInt,
  UInt2,
  UInt3,
  UInt4,
  Int,
  Int2,
  Int3,
  Int4,
  Int1010102Norm,
  UInt1010102Norm,
};
enum class IndexType : uint8_t {
  UInt16,
  UInt32,
};
enum class MultisampleFlags : uint8_t {
  None,
  MS1X,
  MS2X,
  MS4X,
  MS8X,
  MS16X,
};
enum class LoadAction : uint8_t {
  Load,
  Clear,
  DontCare,
};
enum class StoreAction : uint8_t {
  Store,
  DontCare,
};
enum class ComparisonFunction : uint8_t {
  Never,
  Less,
  Equal,
  LessEqual,
  Greater,
  NotEqual,
  GreaterEqual,
  Always,
};
enum class BlendOperation : uint8_t {
  Add,
  Sub,
  RevSub,
  Min,
  Max,
};
enum class BlendFactor : uint8_t {
  Zero,
  One,
  SrcColor,
  OneMinusSrcColor,
  SrcAlpha,
  OneMinusSrcAlpha,
  DestColor,
  OneMinusDestColor,
  DestAlpha,
  OneMinusDestAlpha,
};
enum class StencilOperation : uint8_t {
  Keep,
  Zero,
  Replace,
  IncrementClamp,
  DecrementClamp,
  Invert,
  IncrementWrap,
  DecrementWrap,
};
enum class DepthWriteMask : uint8_t {
  Zero,
  All,
};
enum class ResourceState : uint8_t {
  VertexBuffer,
  UniformBuffer,
  UnorderedAccess,
  SampledImage,
  AccelerationStructure,
};
enum class StorageMode : uint8_t {
  Auto,
  Private,
  Shared,
  Dedicated,
};
enum class TextureDim : uint8_t {
  Tex1D,
  Tex2D,
  Tex3D,
};
enum class FillMode : uint8_t {
  Line,
  Fill,
};
enum class CullMode : uint8_t {
  None,
  Front,
  Back,
};
enum class FilterMode : uint8_t {
  Point,
  Linear,
};
enum class SamplerAddressMode : uint8_t {
  Wrap,
  Mirror,
  Clamp,
  Border,
  MirrorOnce,
};
enum class PrimitiveType : uint8_t {
  Points,
  Lines,
  LineStrips,
  Triangles,
  TriangleStrips,
};
enum class VertexInputRate : uint8_t {
  PerVertex,
  PerInstance,
};
enum class ShaderProfile : uint8_t {
  SM5,
  SM6,
  SM6Raytracing,
};
enum class ShaderOptimizeFlag : uint8_t {
  None = 0,
  StripDebugInfo = 1,
  OptimizeSize = 2,
};
enum class ShaderStage : uint8_t {
  Vertex,
  Pixel,
  Geometry,
  Compute,
  Domain,
  Hull,
  RayGenerate,
  AnyHit,
  ClosetHit,
  MissHit,
  Intersect,
};
enum class RaytracingGeometryFlags : uint8_t {
  None = 0,
  Opaque = 1,
  NoDuplicateAnyHitInvocation = 2,
};
enum class RaytracingGeometryType : uint8_t {
  Triangles,
  AABBs,
};
enum class AccelerationStructureType : uint8_t {
  TopLevel,
  BottomLevel,
};
enum class AccelerationStructureCopyMode : uint8_t {
  Clone,
  Compact,
};
enum class AccelerationStructureBuildFlag : uint8_t {
  None = 0,
  AllowUpdate = 1,
  AllowCompaction = 2,
  PreferFastTrace = 4,
  PreferFastBuild = 8,
  LowMemory = 16,
};
struct DepthStencilOp {
  StencilOperation stencilFailOp;
  StencilOperation depthStencilFailOp;
  StencilOperation stencilPassOp;
  ComparisonFunction stencilFunc;
  uint32 compareMask;
  uint32 writeMask;
  uint32 reference;
};
struct DepthStencilState {
  bool depthTest;
  DepthWriteMask depthWriteMask;
  ComparisonFunction depthFunction;
  bool stencilTest;
  DepthStencilOp front;
  DepthStencilOp back;
};
struct RasterizerState {
  FillMode fillMode;
  CullMode cullMode;
  bool frontCCW;
  bool depthClipEnable;
  float depthBiasClamp;
  float depthBiasSlope;
  float depthBias;
  MultisampleFlags multisample;
};
struct RenderTargetBlendState {
  bool blendEnable;
  BlendFactor srcColor;
  BlendFactor destColor;
  BlendOperation colorOp;
  BlendFactor srcAlpha;
  BlendFactor destAlpha;
  BlendOperation alphaOp;
  uint32 colorWriteMask;
};
struct BlendState {
  Vec<RenderTargetBlendState> renderTargets;
};
struct Rect {
  uint32 x;
  uint32 y;
  uint32 width;
  uint32 height;
};
struct Viewport {
  float x;
  float y;
  float width;
  float height;
  float near;
  float far;
};
struct VertexLayout {
  VertexInputRate inputRate;
  uint32 stride;
};
struct VertexAttribute {
  PixelFormat format;
  uint32 offset;
  uint32 slot;
};
struct VertexInputState {
  Vec<VertexAttribute> attributes;
  Vec<VertexLayout> layouts;
};
enum class BufferUsage : uint8_t {
  VertexBuffer = 1,
  IndexBuffer = 2,
  UniformBuffer = 4,
  UnorderedAccess = 8,
  AccelerationStructure = 16,
};
enum class TextureUsage : uint8_t {
  ShaderResource = 1,
  RenderTarget = 2,
  DepthStencil = 4,
};
struct BufferDesc {
  BufferUsage usages;
  uint32 size;
  uint64 deviceMask;
};
struct TextureDesc {
  TextureUsage usages;
  PixelFormat format;
  uint32 width;
  uint32 height;
  uint32 depth;
  uint32 layers;
  uint32 mipLevels;
  uint64 deviceMask;
};
struct SamplerDesc {
  FilterMode minFilter;
  FilterMode magFilter;
  FilterMode mipMapFilter;
  SamplerAddressMode u;
  SamplerAddressMode v;
  SamplerAddressMode w;
  ComparisonFunction comparisonFunction;
  uint32 maxAnistropy;
  float minLodBias;
  float minLod;
  float maxLod;
};
struct PresentLayerDesc {
  PixelFormat format;
  uint32 width;
  uint32 height;
  ColorSpace colorSpace;
  bool hdrDisplay;
  uint32 maxImages;
  void* winHandle;
  void* extraData;
};
struct Device;
struct LabeledObject : public Rc {
  virtual void setLabel(const char * label) = 0;
  virtual const char * label() const = 0;
};
struct Blob : public Rc {
  virtual const void * data() const = 0;
  virtual uint64 length() const = 0;
};
struct Function : public Rc {
  virtual const char * entry() const = 0;
  virtual const Blob * bundle() const = 0;
};
struct RenderPipelineDesc {
  RasterizerState rasterizer;
  BlendState blend;
  DepthStencilState depthStencil;
  VertexInputState input;
  PixelFormat depthStencilFormat;
  Function* vertex;
  Function* pixel;
  Function* geometry;
  Function* domain;
  Function* hull;
  uint32 deviceMask;
};
struct ComputePipelineDesc {
  Function* function;
  uint32 deviceMask;
};
struct RaytracePipelineDesc {
  uint32 maxTraceRecurseDepth;
  Vec<Function> functions;
};
struct RenderpassDesc {
};
struct Resource : public LabeledObject {
  virtual void * map(uint64 offset, uint64 size) = 0;
  virtual void unmap(void * addr) = 0;
};
struct RaytracingAS : public LabeledObject {
};
struct TextureView;
struct Texture : public Resource {
  virtual PixelFormat format() const = 0;
  virtual TextureView * newView(Result * result) = 0;
};
struct TextureView {
  virtual const Texture * texture() const = 0;
  virtual TextureUsage usage() const = 0;
};
struct BufferView;
struct Buffer : public Resource {
  virtual BufferView * newView(Result * result) = 0;
};
struct BufferView {
  virtual const Buffer * buffer() const = 0;
  virtual BufferUsage usage() const = 0;
};
struct Sampler : public LabeledObject {
};
struct Shader : public LabeledObject {
};
struct Framebuffer : public LabeledObject {
};
struct Drawable;
struct PresentLayer : public Rc {
  virtual void getDesc(PresentLayerDesc * desc) const = 0;
  virtual Device * device() = 0;
  virtual Drawable * nextDrawable() = 0;
};
struct Drawable : public Rc {
  virtual int drawableId() const = 0;
  virtual Texture * texture() = 0;
  virtual PresentLayer * layer() = 0;
  virtual void present() = 0;
};
struct BindGroup : public Rc {
  virtual void setSampler(uint32 id, ShaderStage stage, const Sampler * sampler) = 0;
  virtual void setTexture(uint32 id, ShaderStage stage, const TextureView * texture) = 0;
  virtual void setBuffer(uint32 id, ShaderStage stage, const BufferView * buffer) = 0;
  virtual void setRaytracingAS(uint32 id, ShaderStage stage, const RaytracingAS * as) = 0;
};
struct Pipeline : public LabeledObject {
  virtual BindGroup * newBindGroup(Result * result) = 0;
  virtual Device * device() = 0;
};
struct RenderPipeline : public Pipeline {
};
struct Renderpass : public Rc {
  virtual RenderPipeline * newRenderPipeline(const RenderPipelineDesc * desc, Result * result) = 0;
};
struct ComputePipeline : public Pipeline {
};
struct RaytracePipeline : public Pipeline {
};
struct RenderEncoder;
struct ComputeEncoder;
struct RaytraceEncoder;
struct ParallelEncoder;
struct BlitEncoder;
struct CommandBuffer : public LabeledObject {
  virtual RenderEncoder * newRenderEncoder(Result * result) = 0;
  virtual ComputeEncoder * newComputeEncoder(Result * result) = 0;
  virtual BlitEncoder * newBlitEncoder(Result * result) = 0;
  virtual ParallelEncoder * newParallelRenderEncoder(Result * result) = 0;
  virtual RaytraceEncoder * newRaytraceEncoder(Result * result) = 0;
  virtual Result commit() = 0;
};
struct CommandQueue : public Rc {
  virtual CommandBuffer * newCommandBuffer() = 0;
};
struct CommandEncoder : public LabeledObject {
  virtual void setPipeline(Pipeline * pipeline) = 0;
  virtual void setBindGroup(const BindGroup * bindGroup) = 0;
  virtual void endEncode() = 0;
};
struct RenderEncoder : public CommandEncoder {
  virtual void setViewport(Viewport viewport) = 0;
  virtual void setViewports(int numViewports, const Viewport * pViewport) = 0;
  virtual void setScissors(int numScirssors, const Rect * pRects) = 0;
  virtual void setStencilRef() = 0;
  virtual void setDepthBias() = 0;
  virtual void drawPrimitives(PrimitiveType primType, int vertexStart, int vertexCount, int instanceCount, int baseInstance) = 0;
  virtual void drawIndexedPrimitives(PrimitiveType primType, IndexType indexType, int indexCount, const Buffer * indexBuffer, int indexBufferOffset, int vertexStart, int vertexCount, int instanceCount, int baseInstance) = 0;
  virtual void drawIndirect(PrimitiveType primType, const Buffer * buffer, uint64 offset, uint32 drawCount, uint32 stride) = 0;
  virtual void present(Drawable * drawable) = 0;
};
struct ComputeEncoder : public CommandEncoder {
  virtual void dispatch(int x, int y, int z) = 0;
};
struct ParallelEncoder : public CommandEncoder {
  virtual RenderEncoder * subRenderEncoder(Result * result) = 0;
};
struct BufferStride {
  Buffer* buffer;
  uint32 stride;
};
struct RaytracingAABBs {
  uint32 count;
  BufferStride aabbs;
};
struct RaytracingTriangles {
  uint32 vertexCount;
  BufferStride vertices;
  VertexFormat vertexFormat;
  uint32 indexCount;
  Buffer* indices;
  IndexType indexType;
  Buffer* transforms;
};
struct RaytracingGeometryData {
  RaytracingAABBs aabbs;
  RaytracingTriangles triangles;
};
struct RaytracingGeometryDesc {
  RaytracingGeometryType type;
  RaytracingGeometryFlags flag;
  RaytracingGeometryData data;
};
struct RaytracingASDesc {
  AccelerationStructureType type;
  AccelerationStructureBuildFlag flag;
  uint32 instanceCount;
  uint32 geometryCount;
  const RaytracingGeometryDesc* pGeometries;
};
struct RaytraceEncoder : public CommandEncoder {
  virtual void buildAS(RaytracingAS * src, RaytracingAS * dest, Buffer * scratch) = 0;
  virtual void copyAS(RaytracingAS * src, RaytracingAS * dest, AccelerationStructureCopyMode mode) = 0;
  virtual void traceRay(Buffer * rayGen, BufferStride miss, BufferStride hit, int width, int height) = 0;
};
struct Fence : public LabeledObject {
};
struct Device : public LabeledObject {
  virtual DeviceType getType() const = 0;
  virtual CommandQueue * newQueue(Result * result) = 0;
  virtual Shader * newShader() = 0;
  virtual Renderpass * newRenderpass(const RenderpassDesc * desc, Result * result) = 0;
  virtual ComputePipeline * newComputePipeline(const ComputePipelineDesc * desc, Result * result) = 0;
  virtual RaytracePipeline * newRaytracePipeline(const RaytracePipelineDesc * desc, Result * result) = 0;
  virtual Texture * newTexture(const TextureDesc * desc, StorageMode mode, Result * result) = 0;
  virtual Buffer * newBuffer(const BufferDesc * desc, StorageMode mode, Result * result) = 0;
  virtual RaytracingAS * newRaytracingAS(const RaytracingASDesc * rtDesc, Result * result) = 0;
  virtual Sampler * newSampler(const SamplerDesc * desc, Result * result) = 0;
  virtual Fence * newFence(Result * result) = 0;
  virtual Result wait() = 0;
};
struct Factory : public Rc {
  virtual int numDevices() = 0;
  virtual Device * getDevice(uint32 id) = 0;
  virtual PresentLayer * newPresentLayer(const PresentLayerDesc * desc, Device * device, PresentLayer * old, Result * result) = 0;
};
}

#endif
