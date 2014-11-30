#pragma once
#include "ComputeCommandEncoder.hpp"

class CommandEncoder;
class Drawable;

struct CommandBufferHandler {
    virtual void onCompleted() = 0;
    virtual void onScheduled() = 0;
};

struct RenderPassColorAttachmentDescriptor;
struct RenderPassColorAttachmentDescriptorArray;
struct RenderPassDepthAttachmentDescriptor;
struct RenderPassStencilAttachmentDescriptor;

struct RenderPassDescriptor {
	RenderPassColorAttachmentDescriptorArray *colorAttachmnets;
	RenderPassDepthAttachmentDescriptor * depthAttachment;
	RenderPassStencilAttachmentDescriptor * stencilAttachmnet;
};

class CommandBuffer {
public:

    virtual ~CommandBuffer() {}

//------------ Creating Commander Encoders

    virtual void renderComandEncoderWithDescriptor(RenderPassDescriptor *);

    virtual ComputeCommandEncoder* computeCommandEncoder();

    virtual void blitCommandEncoder();

    virtual void parallelRenderCommandEncoderWithDescriptor();

//------------ Scheduling and Executing Commands

    virtual void enqueue();

    virtual void commit();

    virtual void addScheduledHandler(CommandBufferHandler *);

    virtual void addCompletedHandler(CommandBufferHandler *);

    virtual void presentDrawable();

    virtual void waitUtilScheduled();

    virtual void waitUtilCompleted();

};