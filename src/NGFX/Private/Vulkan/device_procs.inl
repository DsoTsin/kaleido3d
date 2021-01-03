VK_PROTO_FN(GetDeviceQueue);
VK_PROTO_FN(QueueSubmit);
VK_PROTO_FN(QueueWaitIdle);
VK_PROTO_FN(DeviceWaitIdle);

VK_PROTO_FN(AllocateMemory);
VK_PROTO_FN(FreeMemory);
VK_PROTO_FN(MapMemory);
VK_PROTO_FN(UnmapMemory);
VK_PROTO_FN(InvalidateMappedMemoryRanges);
VK_PROTO_FN(FlushMappedMemoryRanges);

VK_PROTO_FN(CreateImage);
VK_PROTO_FN(DestroyImage);
VK_PROTO_FN(BindImageMemory);
VK_PROTO_FN(GetImageMemoryRequirements);
VK_PROTO_FN(CreateImageView);
VK_PROTO_FN(DestroyImageView);

VK_PROTO_FN(CreateBuffer);
VK_PROTO_FN(DestroyBuffer);
VK_PROTO_FN(BindBufferMemory);
VK_PROTO_FN(GetBufferMemoryRequirements);

VK_PROTO_FN(CreateSampler);
VK_PROTO_FN(DestroySampler);

VK_PROTO_FN(CreateFence);
VK_PROTO_FN(DestroyFence);
VK_PROTO_FN(WaitForFences);
VK_PROTO_FN(ResetFences);
VK_PROTO_FN(CreateSemaphore);
VK_PROTO_FN(DestroySemaphore);

VK_PROTO_FN(CreateRenderPass);
VK_PROTO_FN(DestroyRenderPass);
VK_PROTO_FN(CreateFramebuffer);
VK_PROTO_FN(DestroyFramebuffer);

VK_PROTO_FN(CreateShaderModule);
VK_PROTO_FN(DestroyShaderModule);
VK_PROTO_FN(CreateGraphicsPipelines);
VK_PROTO_FN(CreateComputePipelines);
VK_PROTO_FN(DestroyPipeline);
VK_PROTO_FN(CreatePipelineLayout);
VK_PROTO_FN(DestroyPipelineLayout);
VK_PROTO_FN(CreatePipelineCache);
VK_PROTO_FN(DestroyPipelineCache);

VK_PROTO_FN(CreateDescriptorSetLayout);
VK_PROTO_FN(DestroyDescriptorSetLayout);
VK_PROTO_FN(CreateDescriptorPool);
VK_PROTO_FN(DestroyDescriptorPool);
VK_PROTO_FN(AllocateDescriptorSets);
VK_PROTO_FN(FreeDescriptorSets);
VK_PROTO_FN(UpdateDescriptorSets);

// ~ nv raytracing
VK_PROTO_FN(CreateAccelerationStructureNV);
VK_PROTO_FN(DestroyAccelerationStructureNV);
VK_PROTO_FN(GetAccelerationStructureMemoryRequirementsNV);
VK_PROTO_FN(CreateRayTracingPipelinesNV);
VK_PROTO_FN(BindAccelerationStructureMemoryNV);
VK_PROTO_FN(GetAccelerationStructureHandleNV);
VK_PROTO_FN(CmdBuildAccelerationStructureNV);
VK_PROTO_FN(GetRayTracingShaderGroupHandlesNV);
VK_PROTO_FN(CmdTraceRaysNV);
// ~ end nv rt

// ~ debug marker ext
VK_PROTO_FN(DebugMarkerSetObjectNameEXT);
VK_PROTO_FN(CmdDebugMarkerBeginEXT);
VK_PROTO_FN(CmdDebugMarkerEndEXT);
VK_PROTO_FN(CmdDebugMarkerInsertEXT);
// ~ end debug marker ext

// ~ debug util
VK_PROTO_FN(SetDebugUtilsObjectNameEXT);
VK_PROTO_FN(QueueBeginDebugUtilsLabelEXT);
VK_PROTO_FN(QueueInsertDebugUtilsLabelEXT);
VK_PROTO_FN(CmdBeginDebugUtilsLabelEXT);
VK_PROTO_FN(CmdInsertDebugUtilsLabelEXT);
VK_PROTO_FN(CmdEndDebugUtilsLabelEXT);
// ~ end debug util

VK_PROTO_FN(CreateSwapchainKHR);
VK_PROTO_FN(AcquireNextImageKHR);
VK_PROTO_FN(QueuePresentKHR);
VK_PROTO_FN(GetSwapchainImagesKHR);
VK_PROTO_FN(DestroySwapchainKHR);

VK_PROTO_FN(AllocateCommandBuffers);
VK_PROTO_FN(ResetCommandBuffer);
VK_PROTO_FN(FreeCommandBuffers);
VK_PROTO_FN(BeginCommandBuffer);
VK_PROTO_FN(EndCommandBuffer);

VK_PROTO_FN(CreateCommandPool);
VK_PROTO_FN(ResetCommandPool);
VK_PROTO_FN(DestroyCommandPool);

// ~ command recording
VK_PROTO_FN(CmdBeginRenderPass);
VK_PROTO_FN(CmdEndRenderPass);
VK_PROTO_FN(CmdBindPipeline);
VK_PROTO_FN(CmdBindDescriptorSets);
VK_PROTO_FN(CmdBindVertexBuffers);
VK_PROTO_FN(CmdBindIndexBuffer);
VK_PROTO_FN(CmdSetViewport);
VK_PROTO_FN(CmdSetScissor);
VK_PROTO_FN(CmdSetStencilReference);
VK_PROTO_FN(CmdSetDepthBias);
VK_PROTO_FN(CmdDraw);
VK_PROTO_FN(CmdDrawIndexed);
VK_PROTO_FN(CmdDrawIndirect);
VK_PROTO_FN(CmdDispatch);
VK_PROTO_FN(CmdCopyBuffer);
VK_PROTO_FN(CmdCopyImage);
VK_PROTO_FN(CmdCopyBufferToImage);
VK_PROTO_FN(CmdCopyImageToBuffer);
VK_PROTO_FN(CmdBlitImage);
VK_PROTO_FN(CmdPipelineBarrier);
// ~ end command recording