VK_PROTO_FN(GetDeviceQueue);
VK_PROTO_FN(QueueSubmit);

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

VK_PROTO_FN(CreateBuffer);
VK_PROTO_FN(DestroyBuffer);
VK_PROTO_FN(BindBufferMemory);
VK_PROTO_FN(GetBufferMemoryRequirements);

VK_PROTO_FN(CreateFence);
VK_PROTO_FN(DestroyFence);
VK_PROTO_FN(CreateSemaphore);
VK_PROTO_FN(DestroySemaphore);

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

VK_PROTO_FN(CreateCommandPool);
VK_PROTO_FN(ResetCommandPool);
VK_PROTO_FN(DestroyCommandPool);