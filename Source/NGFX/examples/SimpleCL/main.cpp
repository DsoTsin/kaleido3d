/*
SimpleCL SPIRV & OpenCL
*/
#include <Kaleido3D.h>
#include <ngfx.h>
#include <Core/Os.h>

#ifdef REFLECTION
enum class ArgumentAccess
{
  Write = 1,
  Read = 2,
};

enum class ArgumentType
{
  Buffer,
  Texture,
  Sampler,
  ThreadGroupMemory
};

enum DataType
{
  None,
  Struct,
  Array,
  Pointer,
  Sampler,
  Texture,
  Floatx,
};

struct StructMember
{
  char name[64];
  uint32_t offset;
  DataType dataType;
};

struct StructType
{
  StructMember members[1];
  StructMember membderByName;
};

struct PointerType
{
  enum {

  };
  uint32_t alignment;
  uint32_t dataSize;
  DataType elementType;
  bool isArgumentBuffer;
};

struct Argument
{
  char name[128];
  ArgumentAccess access;
  uint32_t index;
  ArgumentType type;
  union
  {
    struct {
      uint32_t bufferAlign;
      uint32_t dataSize;
      DataType dataType;
      // struct Type
    } buffer;
    struct {
      DataType textureType;

    };
  };
};

#endif

#if _WIN32
#pragma comment(linker,"/subsystem:console")
#endif

using namespace ngfx;

void LoadSpv(const char * filePath, void ** ppData, uint32_t * dataSize)
{
  Os::File file(filePath);
  file.Open(IOFlag::IORead);
  *dataSize = file.GetSize();
  char* data = new char[*dataSize];
  file.Read(data, *dataSize);
  file.Close();
  *ppData = data;
}


int main()
{
  Ptr<Factory> factory;
  CreateFactory(factory.GetAddressOf(), true);
  factory->SetName("VulkanFactory");

  Ptr<Device> device;
  uint32_t Count = 0;
  factory->EnumDevice(&Count, nullptr);
  factory->EnumDevice(&Count, device.GetAddressOf());

  Ptr<CommandQueue> queue;
  device->CreateCommandQueue(CommandQueueType::Compute, queue.GetAddressOf());
  queue->SetName("ComputeQueue");

  Ptr<Buffer> buffer;
  BufferDesc bufferDesc{ BufferViewBit::UnOrderedAccess, StorageOption::Managed, 10 };
  device->CreateBuffer(&bufferDesc, buffer.GetAddressOf());
  buffer->SetName("Buffer0");

  Ptr<BufferView> bufferView;
  BufferViewDesc bufferViewDesc = {
    ResourceViewType::UnorderAccessBuffer, 
    ResourceState::UnorderAccess,
    10, 0
  };
  buffer->CreateView(&bufferViewDesc, bufferView.GetAddressOf());

  Ptr<Fence> fence;
  device->CreateFence(fence.GetAddressOf());
  fence->SetName("DefaultFence");

  void* pData = nullptr;
  uint32_t szData = 0;
  LoadSpv("../../Data/Test/SimpleCL.spv", &pData, &szData);

  Ptr<PipelineLayout> pipelineLayout;
  PipelineLayoutDesc pipelineLayoutDesc = {};
  device->CreatePipelineLayout(&pipelineLayoutDesc, pipelineLayout.GetAddressOf());

  Ptr<BindTable> argTable;
  pipelineLayout->CreateBindTable(argTable.GetAddressOf());

  Ptr<Pipeline> computePipeline;
  device->CreateComputePipeline(nullptr, pipelineLayout.Get(), computePipeline.GetAddressOf());

  argTable->SetBuffer(0, ShaderType::Compute, bufferView.Get());

  Ptr<CommandBuffer> commandBuffer;
  Ptr<ComputeCommandEncoder> computeEncoder;
  queue->CreateCommandBuffer(commandBuffer.GetAddressOf());
  commandBuffer->CreateComputeCommandEncoder(computeEncoder.GetAddressOf());
  computeEncoder->SetPipeline(computePipeline.Get());
  computeEncoder->SetBindTable(argTable.Get());
  computeEncoder->Dispatch(1024, 1024, 1);
  computeEncoder->EndEncode();
  commandBuffer->Commit(fence.Get());

  return 0;
}