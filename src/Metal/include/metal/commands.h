#pragma once

#include "enums.h"

namespace MTL {
class Device;

class METAL_API CommandEncoder {
public:
private:
};

class METAL_API ComputeCommandEncoder : public CommandEncoder {
public:
private:
};

class METAL_API RenderCommandEncoder : public CommandEncoder {
public:
private:
};

class METAL_API ParallelRenderCommandEncoder : public CommandEncoder {
public:
private:
};

class METAL_API CommandQueue {
public:
private:
};

class CommandBuffer;
typedef void (*HandlerFunction)(CommandBuffer *);

class METAL_API CommandBuffer {
public:
  CommandQueue *commandQueue() const;
  Device *device() const;

  ComputeCommandEncoder *computeCommandEncoder();
  RenderCommandEncoder *renderCommandEncoder();
  ParallelRenderCommandEncoder *parallelRenderCommandEncoder();

  void encodeWait();
  void encodeSignalEvent();

  void commit();
  void enqueue();

  void addScheduledHandler(HandlerFunction function);
  void addCompletedHandler(HandlerFunction function);

  void waitUtilScheduled();
  void waitUtilCompleted();

  bool retainedReferences() const;

  void pushDebugGroup();
  void popDebugGroup();

private:
};
} // namespace MTL