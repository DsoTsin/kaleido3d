#pragma once
#include "enums.h"
#include "commands.h"
#include "pipeline.h"
#include "resource.h"

namespace MTL {
class Device;

class METAL_API Device {
public:
  CommandQueue *newCommandQueue(uint32_t maxCommandBufferCount);
  Buffer *newBuffer();
  Texture *newTexture();

private:
};
} // namespace MTL

extern "C" METAL_API MTL::Device *MTLCreateSystemDefaultDevice();

//extern "C" NS::Array *MTLCopyAllDevices();