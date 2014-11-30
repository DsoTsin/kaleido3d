#pragma once
#include "CommandBuffer.hpp"

class CommandQueue {
public:
	/** Creates a command buffer. (required)
	*/
	CommandBuffer* commandBuffer();
	CommandBuffer* commandBufferWithUnretainedReferences();

private:

	void enqueue();
	CommandBuffer * dequeue();

private:

	// Atomic Queue
};