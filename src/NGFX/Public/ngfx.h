#ifndef __ngfx_h__
#define __ngfx_h__
#include <ngfx_atomic.h>
#include <ngfx_ptr.h>
#include <ngfx_allocator.h>
#include <ngfx_container.h>
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
  OnGpu,
  OnCpu,
  Shared,
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
enum class IndexType : uint8_t {
  UInt16,
  UInt32,
};
enum class ShaderProfile : uint8_t {
  SM5,
  SM6,
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
  None = (1 << 0),
  Opaque = (1 << 1),
  NoDuplicateAnyHitInvocation = (1 << 2),
};
enum class RaytracingGeometryType : uint8_t {
  Triangles,
  AABBs,
};
enum class AccelerationStructureType : uint8_t {
  TopLevel,
  BottomLevel,
};
enum class AccelerationStructureBuildFlag : uint8_t {
  None = (1 << 0),
  AllowUpdate = (1 << 1),
  AllowCompaction = (1 << 2),
  PreferFastTrace = (1 << 3),
  PreferFastBuild = (1 << 4),
  LowMemory = (1 << 5),
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
  VertexBuffer = (1 << 0),
  IndexBuffer = (1 << 1),
  UniformBuffer = (1 << 2),
  UnorderedAccess = (1 << 3),
  AccelerationStructure = (1 << 4),
};
enum class TextureUsage : uint8_t {
  ShaderResource = (1 << 0),
  RenderTarget = (1 << 1),
  DepthStencil = (1 << 2),
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
struct RaytracingAABBs {
  uint32 count;
};
struct RaytracingTriangles {
  uint32 count;
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
struct Device;
struct LabeledObject : public Rc {
  virtual void set_label(const char * label) = 0;
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
struct Swapchain : public Rc {
  virtual Texture * currentTexture() = 0;
};
struct BindGroup : public Rc {
  virtual void set_sampler(uint32 id, ShaderStage stage, const Sampler * sampler) = 0;
  virtual void set_texture(uint32 id, ShaderStage stage, const TextureView * texture) = 0;
  virtual void set_buffer(uint32 id, ShaderStage stage, const BufferView * buffer) = 0;
  virtual void set_raytracing_as(uint32 id, ShaderStage stage, const RaytracingAS * as) = 0;
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
struct CommandBuffer : public LabeledObject {
  virtual RenderEncoder * newRenderEncoder(Result * result) = 0;
  virtual ComputeEncoder * newComputeEncoder(Result * result) = 0;
  virtual Result newBlitEncoder() = 0;
  virtual Result newParallelRenderEncoder() = 0;
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
  virtual void draw() = 0;
  virtual void present(Swapchain * swapchain) = 0;
};
struct ComputeEncoder : public CommandEncoder {
  virtual void dispatch(int x, int y, int z) = 0;
};
struct RaytraceEncoder : public CommandEncoder {
  virtual void buildAS() = 0;
  virtual void traceRay(int width, int height) = 0;
};
struct Fence : public LabeledObject {
  virtual void signal() = 0;
};
struct Device : public LabeledObject {
  virtual DeviceType getType() const = 0;
  virtual CommandQueue * newQueue() = 0;
  virtual Shader * newShader() = 0;
  virtual Renderpass * newRenderpass(const RenderpassDesc * desc, Result * result) = 0;
  virtual ComputePipeline * newComputePipeline(const ComputePipelineDesc * desc, Result * result) = 0;
  virtual RaytracePipeline * newRaytracePipeline(const RaytracePipelineDesc * desc, Result * result) = 0;
  virtual Texture * newTexture(const TextureDesc * desc, StorageMode mode, Result * result) = 0;
  virtual Buffer * newBuffer(const BufferDesc * desc, StorageMode mode, Result * result) = 0;
  virtual RaytracingAS * newRaytracingAS(const RaytracingASDesc * rtDesc, Result * result) = 0;
  virtual Sampler * newSampler(const SamplerDesc * desc, Result * result) = 0;
  virtual Fence * newFence() = 0;
  virtual Result wait() = 0;
};
struct Factory : public Rc {
  virtual Swapchain * newSwapchain(void * handle, void * reserved) = 0;
  virtual int numDevices() = 0;
  virtual Device * getDevice(uint32 id) = 0;
};
}

#endif