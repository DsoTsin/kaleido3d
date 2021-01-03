VK_PROTO_FN(GetInstanceProcAddr);
VK_PROTO_FN(CreateInstance);
VK_PROTO_FN(DestroyInstance);
VK_PROTO_FN(EnumeratePhysicalDevices);
VK_PROTO_FN(EnumerateInstanceLayerProperties);
VK_PROTO_FN(EnumerateInstanceExtensionProperties);

VK_PROTO_FN(GetPhysicalDeviceQueueFamilyProperties);
VK_PROTO_FN(EnumerateDeviceLayerProperties);
VK_PROTO_FN(EnumerateDeviceExtensionProperties);
VK_PROTO_FN(GetPhysicalDeviceFeatures);
VK_PROTO_FN(GetPhysicalDeviceFeatures2);
VK_PROTO_FN(GetPhysicalDeviceProperties);
VK_PROTO_FN(GetPhysicalDeviceProperties2);
VK_PROTO_FN(GetPhysicalDeviceMemoryProperties);
VK_PROTO_FN(GetPhysicalDeviceMemoryProperties2);

VK_PROTO_FN(CreateDevice);
VK_PROTO_FN(DestroyDevice);
VK_PROTO_FN(GetDeviceProcAddr);

#if defined(VK_KHR_win32_surface)
VK_PROTO_FN(CreateWin32SurfaceKHR);
VK_PROTO_FN(GetPhysicalDeviceWin32PresentationSupportKHR);
#elif defined(VK_KHR_android_surface)
VK_PROTO_FN(CreateAndroidSurfaceKHR);
#endif

VK_PROTO_FN(DestroySurfaceKHR);

VK_PROTO_FN(GetPhysicalDeviceSurfaceSupportKHR);
VK_PROTO_FN(GetPhysicalDeviceSurfaceCapabilitiesKHR);
VK_PROTO_FN(GetPhysicalDeviceSurfaceFormatsKHR);
VK_PROTO_FN(GetPhysicalDeviceSurfacePresentModesKHR);

VK_PROTO_FN(CreateDebugReportCallbackEXT);
VK_PROTO_FN(DestroyDebugReportCallbackEXT);

// Need HDR support ?
VK_PROTO_FN(SetHdrMetadataEXT);