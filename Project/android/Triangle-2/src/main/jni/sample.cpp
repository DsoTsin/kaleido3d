//                  Copyright (c) 2016 QUALCOMM Technologies Inc.
//                              All Rights Reserved.

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#include "sample.h"
#include "shader.h"
#include "native_app_glue/android_native_app_glue.h"

// The vertex buffer bind id, used as a constant in various places in the sample
#define VERTEX_BUFFER_BIND_ID 0

// Sample Name
#define SAMPLE_NAME "Vulkan Triangle Sample"

using namespace std;
using namespace vk;

VkSample::VkSample()
{
    m_initialized = false;
    m_initBegun = false;
}
///////////////////////////////////////////////////////////////////////////////

VkSample::~VkSample()
{
    
    if (m_initBegun)
    {
        TearDown();
    }

}

///////////////////////////////////////////////////////////////////////////////

/**
 * VkSample::Initialize
 *
 * Initializes the Vulkan subsystem to a default sample state.
 */
bool VkSample::Initialize(ANativeWindow* window)
{
    VkResult ret = VK_SUCCESS;

    if (m_initBegun)
    {
        TearDown();
    }

    //m_initBegun acts as a signal that a partial teardown may be
    //needed, regardless of m_initialized state.
    m_initBegun = true;

    //The android window to render to is passed, we must use this
    //later in the initialization sequence.
    m_androidWindow = window;

    CreateInstance();
    GetPhysicalDevices();

    InitDevice();
    InitSwapchain();
    InitCommandbuffers();
    InitVertexBuffers();
    InitLayouts();
    InitRenderPass();
    InitPipeline();
    InitFrameBuffers();
    InitSync();

    // Initialize our command buffers
    BuildCmdBuffer();

    // We acquire the next swap chain image, in preparation for the render loop
    SetNextBackBuffer();

    m_frameIdx = 0;
    m_frameTimeBegin = GetTime();

    m_initialized = true;
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool VkSample::CreateInstance()
{
    VkResult ret = VK_SUCCESS;  

    // Discover the number of extensions listed in the instance properties in order to allocate
    // a buffer large enough to hold them.
//    uint32_t instanceExtensionCount = 0;
//    ret = vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionCount, nullptr);
//    VK_CHECK(!ret);

    VkBool32 surfaceExtFound = 0;
    VkBool32 platformSurfaceExtFound = 0;
//    VkExtensionProperties* instanceExtensions = nullptr;
//    instanceExtensions = new VkExtensionProperties[instanceExtensionCount];

    // Now request instanceExtensionCount VkExtensionProperties elements be read into out buffer
//    ret = vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionCount, instanceExtensions);
//    VK_CHECK(!ret);

    // We require two extensions, VK_KHR_surface and VK_KHR_android_surface. If they are found,
    // add them to the extensionNames list that we'll use to initialize our instance with later.
    uint32_t enabledExtensionCount = 0;
    const char* extensionNames[16];

//    for (uint32_t i = 0; i < instanceExtensionCount; i++) {
//        if (!strcmp(VK_KHR_SURFACE_EXTENSION_NAME, instanceExtensions[i].extensionName)) {
//            surfaceExtFound = 1;
//            extensionNames[enabledExtensionCount++] = VK_KHR_SURFACE_EXTENSION_NAME;
//        }
//
//        if (!strcmp(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME, instanceExtensions[i].extensionName)) {
//            platformSurfaceExtFound = 1;
//            extensionNames[enabledExtensionCount++] = VK_KHR_ANDROID_SURFACE_EXTENSION_NAME;
//        }
//        VK_CHECK(enabledExtensionCount < 16);
//    }

    std::vector<vk::ExtensionProperties> props = vk::enumerateInstanceExtensionProperties(nullptr);

    for (auto &prop : props) {
        if (!strcmp(VK_KHR_SURFACE_EXTENSION_NAME, prop.extensionName)) {
            surfaceExtFound = 1;
            extensionNames[enabledExtensionCount++] = VK_KHR_SURFACE_EXTENSION_NAME;
        }

        if (!strcmp(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME, prop.extensionName)) {
            platformSurfaceExtFound = 1;
            extensionNames[enabledExtensionCount++] = VK_KHR_ANDROID_SURFACE_EXTENSION_NAME;
        }
        VK_CHECK(enabledExtensionCount < 16);
    }

    if (!surfaceExtFound) {
        LOGE("vkEnumerateInstanceExtensionProperties failed to find the " VK_KHR_SURFACE_EXTENSION_NAME" extension.");
        return false;
    }
    if (!platformSurfaceExtFound) {
        LOGE("vkEnumerateInstanceExtensionProperties failed to find the " VK_KHR_ANDROID_SURFACE_EXTENSION_NAME" extension.");
        return false;
    }

    // We specify the Vulkan version our application was built with,
    // as well as names and versions for our application and engine,
    // if applicable. This allows the driver to gain insight to what
    // is utilizing the vulkan driver, and serve appropriate versions.
//    VkApplicationInfo applicationInfo = {};
//    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
//    applicationInfo.pNext = nullptr;
//    applicationInfo.pApplicationName = SAMPLE_NAME;
//    applicationInfo.applicationVersion = 0;
//    applicationInfo.pEngineName = "VkSample";
//    applicationInfo.engineVersion = 1;
//    applicationInfo.apiVersion = VK_MAKE_VERSION(1,0,0);

    vk::ApplicationInfo appInfo(SAMPLE_NAME, 0, "VkSample", 1, VK_MAKE_VERSION(1,0,0));

    // Creation information for the instance points to details about
    // the application, and also the list of extensions to enable.
//    VkInstanceCreateInfo instanceCreateInfo = {};
//    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
//    instanceCreateInfo.pNext = nullptr;
//    instanceCreateInfo.pApplicationInfo = &applicationInfo;
//    instanceCreateInfo.enabledLayerCount = 0;
//    instanceCreateInfo.ppEnabledLayerNames = nullptr;
//    instanceCreateInfo.enabledExtensionCount = enabledExtensionCount;
//    instanceCreateInfo.ppEnabledExtensionNames = extensionNames;

    vk::InstanceCreateInfo instCreateInfo;
    instCreateInfo.setPApplicationInfo(&appInfo)
            .setEnabledExtensionCount(enabledExtensionCount)
            .setPpEnabledExtensionNames(extensionNames);


    // The main Vulkan instance is created with the creation infos above.
    // We do not specify a custom memory allocator for instance creation.
//    ret = vkCreateInstance(&instanceCreateInfo, nullptr, &m_instance);
//    vk::createInstance()
    // we can delete the list of extensions after calling vkCreateInstance
//    delete[] instanceExtensions;
    instance = vk::createInstance(instCreateInfo);

    // Vulkan API return values can expose further information on a failure.
    // For instance, INCOMPATIBLE_DRIVER may be returned if the API level
    // an application is built with, exposed through VkApplicationInfo, is 
    // newer than the driver present on a device.
//    if (ret == VK_ERROR_INCOMPATIBLE_DRIVER) {
//        LOGE("Cannot find a compatible Vulkan installable client driver: vkCreateInstance Failure");
//        return false;
//    } else if (ret == VK_ERROR_EXTENSION_NOT_PRESENT) {
//        LOGE("Cannot find a specified extension library: vkCreateInstance Failure");
//        return false;
//    } else {
//        VK_CHECK(!ret);
//    }
     
    return true;
}
///////////////////////////////////////////////////////////////////////////////

bool VkSample::GetPhysicalDevices()
{
    VkResult ret = VK_SUCCESS;

    // Query number of physical devices available
//    ret = vkEnumeratePhysicalDevices(m_instance, &m_physicalDeviceCount, nullptr);
//    VK_CHECK(!ret);
//
//    if (m_physicalDeviceCount == 0)
//    {
//        LOGE("No physical devices detected.");
//        return false;
//    }

    // Allocate space the the correct number of devices, before requesting their data
//    m_pPhysicalDevices = new VkPhysicalDevice[m_physicalDeviceCount];
//    ret = vkEnumeratePhysicalDevices(m_instance, &m_physicalDeviceCount, m_pPhysicalDevices);
//    VK_CHECK(!ret);

    std::vector<vk::PhysicalDevice> devices = instance.enumeratePhysicalDevices();
    physicalDevice = devices[0];

    // For purposes of this sample, we simply use the first device.
//    m_physicalDevice = m_pPhysicalDevices[0];

    // By querying the device properties, we learn the device name, amongst
    // other details.
//    vkGetPhysicalDeviceProperties(m_physicalDevice, &m_physicalDeviceProperties);

    physicalDeviceProps = physicalDevice.getProperties();

    LOGI("Vulkan Device: %s", physicalDeviceProps.deviceName);

    physicalDeviceMemoryProps = physicalDevice.getMemoryProperties();
    // Get Memory information and properties - this is required later, when we begin
    // allocating buffers to store data.
//    vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &m_physicalDeviceMemoryProperties);

    return true;
}

///////////////////////////////////////////////////////////////////////////////

void VkSample::InitDevice()
{
    VkResult ret = VK_SUCCESS;
    // Akin to when creating the instance, we can query extensions supported by the physical device
    // that we have selected to use.
//    uint32_t deviceExtensionCount = 0;
//    VkExtensionProperties *device_extensions = nullptr;
//    ret = vkEnumerateDeviceExtensionProperties(m_physicalDevice, nullptr, &deviceExtensionCount, nullptr);
//    VK_CHECK(!ret);

    std::vector<vk::ExtensionProperties> extProps = physicalDevice.enumerateDeviceExtensionProperties();

    VkBool32 swapchainExtFound = 0;
//    VkExtensionProperties* deviceExtensions = new VkExtensionProperties[deviceExtensionCount];
//    ret = vkEnumerateDeviceExtensionProperties(m_physicalDevice, nullptr, &deviceExtensionCount, deviceExtensions);
//    VK_CHECK(!ret);

    // For our example, we require the swapchain extension, which is used to present backbuffers efficiently
    // to the users screen.
    uint32_t enabledExtensionCount = 0;
    const char* extensionNames[16] = {0};
    for (uint32_t i = 0; i < extProps.size(); i++) {
        if (!strcmp(VK_KHR_SWAPCHAIN_EXTENSION_NAME, extProps[i].extensionName)) {
            swapchainExtFound = 1;
            extensionNames[enabledExtensionCount++] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
        }
        VK_CHECK(enabledExtensionCount < 16);
    }
    if (!swapchainExtFound) {
        LOGE("vkEnumerateDeviceExtensionProperties failed to find the " VK_KHR_SWAPCHAIN_EXTENSION_NAME " extension: vkCreateInstance Failure");

        // Always attempt to enable the swapchain
        extensionNames[enabledExtensionCount++] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
    }

    InitSurface();

    // Before we create our main Vulkan device, we must ensure our physical device
    // has queue families which can perform the actions we require. For this, we request
    // the number of queue families, and their properties.
//    uint32_t queueFamilyCount = 0;
//    vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, nullptr);

    vector<QueueFamilyProperties> queueFamilyProps = physicalDevice.getQueueFamilyProperties();


//    VkQueueFamilyProperties* queueProperties = new VkQueueFamilyProperties[queueFamilyCount];
//    vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, queueProperties);
//    VK_CHECK(queueFamilyCount >= 1);

    // We query each queue family in turn for the ability to support the android surface
    // that was created earlier. We need the device to be able to present its images to
    // this surface, so it is important to test for this.
    VkBool32* supportsPresent = new VkBool32[queueFamilyProps.size()];
    for (uint32_t i = 0; i < queueFamilyProps.size(); i++) {
//        vkGetPhysicalDeviceSurfaceSupportKHR(m_physicalDevice, i,  m_surface, &supportsPresent[i]);
        supportsPresent[i] = physicalDevice.getSurfaceSupportKHR(i, surface);
    }



    // Search for a graphics queue, and ensure it also supports our surface. We want a
    // queue which can be used for both, as to simplify operations.
    uint32_t queueIndex = UINT32_MAX;
    for (uint32_t i = 0; i < queueFamilyProps.size(); i++) {
        if ((uint32_t)(QueueFlagBits::eGraphics & queueFamilyProps[i].queueFlags) != 0) {
            if (supportsPresent[i] == VK_TRUE) {
                queueIndex = i;
                break;
            }
        }
    }

    delete [] supportsPresent;
//    delete [] queueProperties;

    if (queueIndex == UINT32_MAX) {
        VK_CHECK("Could not obtain a queue family for both graphics and presentation." && 0);
    }

    // We have identified a queue family which both supports our android surface,
    // and can be used for graphics operations.
    m_queueFamilyIndex = queueIndex;


    // As we create the device, we state we will be creating a queue of the
    // family type required. 1.0 is the highest priority and we use that.
    float queuePriorities[1] = { 1.0 };
//    VkDeviceQueueCreateInfo deviceQueueCreateInfo = {};
//    deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
//    deviceQueueCreateInfo.pNext = nullptr;
//    deviceQueueCreateInfo.queueFamilyIndex = m_queueFamilyIndex;
//    deviceQueueCreateInfo.queueCount = 1;
//    deviceQueueCreateInfo.pQueuePriorities = queuePriorities;

    vk::DeviceQueueCreateInfo queueCreateInfo;
    queueCreateInfo.setQueueCount(1).setQueueFamilyIndex(m_queueFamilyIndex).setPQueuePriorities(queuePriorities);

    // Now we pass the queue create info, as well as our requested extensions,
    // into our DeviceCreateInfo structure.
//    VkDeviceCreateInfo deviceCreateInfo = {};
//    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
//    deviceCreateInfo.pNext = nullptr;
//    deviceCreateInfo.queueCreateInfoCount = 1;
//    deviceCreateInfo.pQueueCreateInfos = &deviceQueueCreateInfo;
//    deviceCreateInfo.enabledLayerCount = 0;
//    deviceCreateInfo.ppEnabledLayerNames = nullptr;
//    deviceCreateInfo.enabledExtensionCount = enabledExtensionCount;
//    deviceCreateInfo.ppEnabledExtensionNames = extensionNames;

    vk::DeviceCreateInfo deviceCreateInfo;
    deviceCreateInfo.setQueueCreateInfoCount(1).setPQueueCreateInfos(&queueCreateInfo)
            .setEnabledExtensionCount(enabledExtensionCount).setPpEnabledExtensionNames(extensionNames);


    // Create the device.
//    ret = vkCreateDevice(m_physicalDevice, &deviceCreateInfo, nullptr, &m_device);
//    VK_CHECK(!ret);

    device = physicalDevice.createDevice(deviceCreateInfo);

    // Obtain the device queue that we requested.
//    vkGetDeviceQueue(m_device, m_queueFamilyIndex, 0, &m_queue);
    queue = device.getQueue(m_queueFamilyIndex,0);
}

///////////////////////////////////////////////////////////////////////////////

void VkSample::InitSurface()
{
//    VkResult ret = VK_SUCCESS;
    // At this point, we create the android surface. This is because we want to
    // ensure our device is capable of working with the created surface object.
//    VkAndroidSurfaceCreateInfoKHR surfaceCreateInfo = {};
//    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
//    surfaceCreateInfo.pNext = nullptr;
//    surfaceCreateInfo.flags = 0;
//    surfaceCreateInfo.window = m_androidWindow;

    vk::AndroidSurfaceCreateInfoKHR surfaceCreateInfoKHR;
    surfaceCreateInfoKHR.setWindow(m_androidWindow);

//    ret = vkCreateAndroidSurfaceKHR(m_instance, &surfaceCreateInfo, nullptr, &m_surface);
//    VK_CHECK(!ret);

    surface = instance.createAndroidSurfaceKHR(surfaceCreateInfoKHR);
}

///////////////////////////////////////////////////////////////////////////////

void VkSample::InitSwapchain()
{
//    VkResult ret = VK_SUCCESS;
    // By querying the supported formats of our surface, we can ensure that
    // we use one that the device can work with.
//    uint32_t formatCount;
//    ret = vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &formatCount, nullptr);
//    VK_CHECK(!ret);

//    VkSurfaceFormatKHR* surfFormats = new VkSurfaceFormatKHR[formatCount];
//    ret = vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &formatCount, surfFormats);
//    VK_CHECK(!ret);

    vector<SurfaceFormatKHR> surfaceFmts = physicalDevice.getSurfaceFormatsKHR(surface);


    // If the format list includes just one entry of VK_FORMAT_UNDEFINED,
    // the surface has no preferred format.  Otherwise, at least one
    // supported format will be returned. For the purposes of this sample, 
    // we use the first format returned.
    if (surfaceFmts.size() == 1 && surfaceFmts[0].format == Format::eUndefined)
    {
        surfaceFormat.format = Format::eB8G8R8A8Unorm;
        surfaceFormat.colorSpace = surfaceFmts[0].colorSpace;
    }
    else
    {
        surfaceFormat = surfaceFmts[0];
    }

//    delete [] surfFormats;


    // Now we obtain the surface capabilities, which contains details such as width and height.
//    VkSurfaceCapabilitiesKHR surfaceCapabilities;
//    ret = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physicalDevice, m_surface, &surfaceCapabilities);
//    VK_CHECK(!ret);

    SurfaceCapabilitiesKHR surfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface);

    m_width = surfaceCapabilities.currentExtent.width;
    m_height = surfaceCapabilities.currentExtent.height;

    // Now that we have selected formats and obtained ideal surface dimensions,
    // we create the swapchain. We use FIFO mode, which is always present. This
    // mode has a queue of images internally, that will be presented to the screen.
    // The swapchain will be created and expose the number of images created
    // in the queue, which will be at least the number specified in minImageCount.
//    VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
//    swapchainCreateInfo.sType                 = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
//    swapchainCreateInfo.surface               = m_surface;
//    swapchainCreateInfo.minImageCount         = surfaceCapabilities.minImageCount;
//    swapchainCreateInfo.imageFormat           = m_surfaceFormat.format;
//    swapchainCreateInfo.imageColorSpace       = m_surfaceFormat.colorSpace;
//    swapchainCreateInfo.imageExtent.width     = m_width;
//    swapchainCreateInfo.imageExtent.height    = m_height;
//    swapchainCreateInfo.imageUsage            = surfaceCapabilities.supportedUsageFlags;
//    swapchainCreateInfo.preTransform          = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
//    swapchainCreateInfo.imageArrayLayers      = 1;
//    swapchainCreateInfo.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
//    swapchainCreateInfo.compositeAlpha        = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
//    swapchainCreateInfo.presentMode           = VK_PRESENT_MODE_FIFO_KHR;
//    swapchainCreateInfo.clipped               = VK_TRUE;

//    ret = vkCreateSwapchainKHR(m_device, &swapchainCreateInfo, nullptr, &m_swapchain);
//    VK_CHECK(!ret);

    SwapchainCreateInfoKHR swapchainCreateInfoKHR;
    swapchainCreateInfoKHR.setSurface(surface).setMinImageCount(surfaceCapabilities.minImageCount)
            .setImageFormat(surfaceFormat.format).setImageColorSpace(surfaceFormat.colorSpace)
            .setImageExtent(Extent2D(m_width, m_height)).setImageUsage(surfaceCapabilities.supportedUsageFlags)
            .setPreTransform(SurfaceTransformFlagBitsKHR::eIdentity).setImageArrayLayers(1)
            .setImageSharingMode(SharingMode::eExclusive).setCompositeAlpha(CompositeAlphaFlagBitsKHR::eOpaque)
            .setPresentMode(PresentModeKHR::eFifo).setClipped(VK_TRUE);

    swapChain = device.createSwapchainKHR(swapchainCreateInfoKHR);
    // Query the number of swapchain images. This is the number of images in the internal
    // queue.
//    ret = vkGetSwapchainImagesKHR(m_device, m_swapchain, &m_swapchainImageCount, nullptr);
//    VK_CHECK(!ret);


    // Now we can retrieve these images, as to use them in rendering as our framebuffers.
//    VkImage* pSwapchainImages = new VkImage[m_swapchainImageCount];
//    ret = vkGetSwapchainImagesKHR(m_device, m_swapchain, &m_swapchainImageCount, pSwapchainImages);
//    VK_CHECK(!ret);

    vector<Image> swapChainImages = device.getSwapchainImagesKHR(swapChain);
    m_swapchainImageCount = swapChainImages.size();
    LOGI("Swapchain Image Count: %d\n", m_swapchainImageCount);
    // We prepare our own representation of the swapchain buffers, for keeping track
    // of resources during rendering.
    m_swapchainBuffers = new SwapchainBuffer[m_swapchainImageCount];
    VK_CHECK(m_swapchainBuffers);


    // From the images obtained from the swapchain, we create image views.
    // This gives us context into the image.
//    VkImageViewCreateInfo imageViewCreateInfo = {};
//    imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
//    imageViewCreateInfo.pNext = nullptr;
//    imageViewCreateInfo.format = m_surfaceFormat.format;
//    imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_R;
//    imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_G;
//    imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_B;
//    imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_A;
//    imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//    imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
//    imageViewCreateInfo.subresourceRange.levelCount = 1;
//    imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
//    imageViewCreateInfo.subresourceRange.layerCount = 1;
//    imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
//    imageViewCreateInfo.flags = 0;

    ImageViewCreateInfo viewCreateInfo;
    viewCreateInfo.setFormat(surfaceFormat.format).setSubresourceRange(ImageSubresourceRange(ImageAspectFlagBits::eColor,0,1,0,1))
            .setComponents(ComponentMapping(ComponentSwizzle::eR,ComponentSwizzle::eG,ComponentSwizzle::eB,ComponentSwizzle::eA))
            .setViewType(ImageViewType::e2D);

    for (uint32_t i = 0; i < m_swapchainImageCount; i++) {
        // We create an Imageview for each swapchain image, and track
        // the view and image in our swapchainBuffers object.
        m_swapchainBuffers[i].image = swapChainImages[i];
        viewCreateInfo.image = swapChainImages[i];

//        VkResult err = vkCreateImageView(m_device, &, nullptr, &m_swapchainBuffers[i].view);
//        VK_CHECK(!err);
        m_swapchainBuffers[i].view = device.createImageView(viewCreateInfo);
    }

    // At this point, we have the references now in our swapchainBuffer object
//    delete [] pSwapchainImages;


    // Now we create depth buffers for our swapchain images, which form part of
    // our framebuffers later.
    m_depthBuffers = new DepthBuffer[m_swapchainImageCount];
    for (int i = 0; i < m_swapchainImageCount; i++) {
        const VkFormat depthFormat = VK_FORMAT_D16_UNORM;

        const Format depthFmt = Format::eD16Unorm;

//        VkImageCreateInfo imageCreateInfo = {};
//        imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
//        imageCreateInfo.pNext = nullptr;
//        imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
//        imageCreateInfo.format = depthFormat;
//        imageCreateInfo.extent = {m_width, m_height, 1};
//        imageCreateInfo .mipLevels = 1;
//        imageCreateInfo .arrayLayers = 1;
//        imageCreateInfo .samples = VK_SAMPLE_COUNT_1_BIT;
//        imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
//        imageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
//        imageCreateInfo .flags = 0;

        ImageCreateInfo imgCreateInfo;
        imgCreateInfo.setImageType(ImageType::e2D).setFormat(depthFmt).setMipLevels(1)
                .setArrayLayers(1).setSamples(SampleCountFlagBits::e1).setTiling(ImageTiling::eOptimal)
                .setUsage(ImageUsageFlagBits::eDepthStencilAttachment)
                .setExtent(Extent3D(m_width,m_height,1));

//        VkImageViewCreateInfo imageViewCreateInfo = {};
//        imageViewCreateInfo .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
//        imageViewCreateInfo .pNext = nullptr;
//        imageViewCreateInfo .image = VK_NULL_HANDLE;
//        imageViewCreateInfo.format = depthFormat;
//        imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
//        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
//        imageViewCreateInfo.subresourceRange.levelCount = 1;
//        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
//        imageViewCreateInfo.subresourceRange.layerCount = 1;
//        imageViewCreateInfo.flags = 0;
//        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;

        ImageViewCreateInfo imgViewCreateInfo;
        imgViewCreateInfo.setFormat(depthFmt)
                .setSubresourceRange(ImageSubresourceRange(ImageAspectFlagBits::eDepth,0,1,0,1))
                .setViewType(ImageViewType::e2D);

//        VkMemoryRequirements mem_reqs;
//        VkResult  err;
        bool  pass;

        m_depthBuffers[i].format = depthFmt;

        // Create the image with details as imageCreateInfo
//        err = vkCreateImage(m_device, &imageCreateInfo, nullptr, &m_depthBuffers[i].image);
//        VK_CHECK(!err);

        m_depthBuffers[i].image = device.createImage(imgCreateInfo);

        // discover what memory requirements are for this image.
//        vkGetImageMemoryRequirements(m_device, m_depthBuffers[i].image, &mem_reqs);

        MemoryRequirements memoryRequirements = device.getImageMemoryRequirements(m_depthBuffers[i].image);

        // Allocate memory according to requirements
//        VkMemoryAllocateInfo memoryAllocateInfo = {};
//        memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
//        memoryAllocateInfo.pNext = nullptr;
//        memoryAllocateInfo.allocationSize = 0;
//        memoryAllocateInfo.memoryTypeIndex = 0;
//        memoryAllocateInfo.allocationSize = mem_reqs.size;
//        pass = GetMemoryTypeFromProperties(mem_reqs.memoryTypeBits, 0, &memoryAllocateInfo.memoryTypeIndex);
//        VK_CHECK(pass);

        MemoryAllocateInfo memoryAllocateInfo(memoryRequirements.size, 0);
        pass = GetMemoryTypeFromProperties(memoryRequirements.memoryTypeBits, (MemoryPropertyFlagBits)0, &memoryAllocateInfo.memoryTypeIndex);


//        err = vkAllocateMemory(m_device, &memoryAllocateInfo, nullptr, &m_depthBuffers[i].mem);
//        VK_CHECK(!err);

        m_depthBuffers[i].mem = device.allocateMemory(memoryAllocateInfo);

        // Bind memory to the image
//        err = vkBindImageMemory(m_device, m_depthBuffers[i].image, m_depthBuffers[i].mem, 0);
//        VK_CHECK(!err);

        device.bindImageMemory(m_depthBuffers[i].image, m_depthBuffers[i].mem, 0);

        // Create the view for this image
        imgViewCreateInfo.image = m_depthBuffers[i].image;
//        err = vkCreateImageView(m_device, &imageViewCreateInfo, nullptr, &m_depthBuffers[i].view);
//        VK_CHECK(!err);
        m_depthBuffers[i].view = device.createImageView(imgViewCreateInfo);
    }

}

///////////////////////////////////////////////////////////////////////////////

void VkSample::InitCommandbuffers()
{
    VkResult ret = VK_SUCCESS;
    // Command buffers are allocated from a pool; we define that pool here and create it.
//    VkCommandPoolCreateInfo commandPoolCreateInfo = {};
//    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
//    commandPoolCreateInfo.pNext = nullptr;
//    commandPoolCreateInfo.queueFamilyIndex = m_queueFamilyIndex;
//    commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

//    ret = vkCreateCommandPool(m_device, &commandPoolCreateInfo, nullptr, &m_commandPool);
//    VK_CHECK(!ret);

    CommandPoolCreateInfo commandPoolCreateInfo(CommandPoolCreateFlagBits::eResetCommandBuffer, m_queueFamilyIndex);
    commandPool = device.createCommandPool(commandPoolCreateInfo);

//    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
//    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
//    commandBufferAllocateInfo.pNext = nullptr;
//    commandBufferAllocateInfo.commandPool = m_commandPool;
//    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
//    commandBufferAllocateInfo.commandBufferCount = 1;

    CommandBufferAllocateInfo commandBufferAllocateInfo(commandPool, CommandBufferLevel::ePrimary,1);
    // Create render command buffers, one per swapchain image
    for (int i=0; i < m_swapchainImageCount; i++)
    {
//        ret = vkAllocateCommandBuffers(m_device, &commandBufferAllocateInfo, &m_swapchainBuffers[i].cmdBuffer);
//        VK_CHECK(!ret);
        m_swapchainBuffers[i].cmdBuffer = device.allocateCommandBuffers(commandBufferAllocateInfo)[0];
    }

}

///////////////////////////////////////////////////////////////////////////////

void VkSample::InitVertexBuffers()
{
    // Our vertex buffer data is a simple triangle, with associated vertex colors.
    const float vb[3][7] = {
            //      position                   color
            { -0.9f, -0.9f,  0.9f,     1.0f, 0.0f, 0.0f, 1.0f },
            {  0.9f, -0.9f,  0.9f,     0.0f, 1.0f, 0.0f, 1.0f },
            {  0.0f,  0.9f,  0.9f,     0.0f, 0.0f, 1.0f, 1.0f },
    };

    VkResult   err;
    bool   pass;

    // Our m_vertices member contains the types required for storing
    // and defining our vertex buffer within the graphics pipeline.
    memset(&m_vertices, 0, sizeof(m_vertices));

    // Create our buffer object.
//    VkBufferCreateInfo bufferCreateInfo = {};
//    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
//    bufferCreateInfo.pNext = nullptr;
//    bufferCreateInfo.size = sizeof(vb);
//    bufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
//    bufferCreateInfo.flags = 0;
//    err = vkCreateBuffer(m_device, &bufferCreateInfo, nullptr, &m_vertices.buf);
//    VK_CHECK(!err);

    BufferCreateInfo bufCreateInfo;
    bufCreateInfo.setUsage(BufferUsageFlagBits::eVertexBuffer).setSize(sizeof(vb));
    m_vertices.buf = device.createBuffer(bufCreateInfo);

    // Obtain the memory requirements for this buffer.
//    VkMemoryRequirements mem_reqs;
//    vkGetBufferMemoryRequirements(m_device, m_vertices.buf, &mem_reqs);
//    VK_CHECK(!err);

    MemoryRequirements memoryRequirements = device.getBufferMemoryRequirements(m_vertices.buf);

    // And allocate memory according to those requirements.
//    VkMemoryAllocateInfo memoryAllocateInfo = {};
//    memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
//    memoryAllocateInfo.pNext = nullptr;
//    memoryAllocateInfo.allocationSize = 0;
//    memoryAllocateInfo.memoryTypeIndex = 0;
//    memoryAllocateInfo.allocationSize  = mem_reqs.size;
//    pass = GetMemoryTypeFromProperties(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &memoryAllocateInfo.memoryTypeIndex);
//    VK_CHECK(pass);

    MemoryAllocateInfo memAllocInfo(memoryRequirements.size,0);
    pass = GetMemoryTypeFromProperties(memoryRequirements.memoryTypeBits, MemoryPropertyFlagBits::eHostVisible, &memAllocInfo.memoryTypeIndex);

//    err = vkAllocateMemory(m_device, &memoryAllocateInfo, nullptr, &m_vertices.mem);
//    VK_CHECK(!err);
    m_vertices.mem = device.allocateMemory(memAllocInfo);

    // Now we need to map the memory of this new allocation so the CPU can edit it.
    void *data = device.mapMemory(m_vertices.mem, 0, memAllocInfo.allocationSize);
//    VK_CHECK(!err);

    // Copy our triangle verticies and colors into the mapped memory area.
    memcpy(data, vb, sizeof(vb));

    // Unmap the memory back from the CPU.
//    vkUnmapMemory(m_device, m_vertices.mem);
    device.unmapMemory(m_vertices.mem);

    // Bind our buffer to the memory.
//    err = vkBindBufferMemory(m_device, m_vertices.buf, m_vertices.mem, 0);
//    VK_CHECK(!err);
    device.bindBufferMemory(m_vertices.buf, m_vertices.mem, 0);

    // The vertices need to be defined so that the pipeline understands how the
    // data is laid out. This is done by providing a VkPipelineVertexInputStateCreateInfo
    // structure with the correct information.
//    m_vertices.vi.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
//    m_vertices.vi.pNext = nullptr;
//    m_vertices.vi.vertexBindingDescriptionCount = 1;
//    m_vertices.vi.pVertexBindingDescriptions = m_vertices.vi_bindings;
//    m_vertices.vi.vertexAttributeDescriptionCount = 2;
//    m_vertices.vi.pVertexAttributeDescriptions = m_vertices.vi_attrs;

    m_vertices.vi.setVertexBindingDescriptionCount(1).setVertexAttributeDescriptionCount(2)
            .setPVertexBindingDescriptions(m_vertices.vi_bindings)
            .setPVertexAttributeDescriptions(m_vertices.vi_attrs);

    // We bind the buffer as a whole, using the correct buffer ID.
    // This defines the stride for each element of the vertex array.
//    m_vertices.vi_bindings[0].binding = VERTEX_BUFFER_BIND_ID;
//    m_vertices.vi_bindings[0].stride = sizeof(vb[0]);
//    m_vertices.vi_bindings[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    m_vertices.vi_bindings[0].setBinding(VERTEX_BUFFER_BIND_ID).setStride(sizeof(vb[0])).setInputRate(VertexInputRate::eVertex);

    // Within each element, we define the attributes. At location 0,
    // the vertex positions, in float3 format, with offset 0 as they are
    // first in the array structure.
//    m_vertices.vi_attrs[0].binding = VERTEX_BUFFER_BIND_ID;
//    m_vertices.vi_attrs[0].location = 0;
//    m_vertices.vi_attrs[0].format = VK_FORMAT_R32G32B32_SFLOAT; //float3
//    m_vertices.vi_attrs[0].offset = 0;

    m_vertices.vi_attrs[0].setBinding(VERTEX_BUFFER_BIND_ID)
            .setLocation(0)
            .setFormat(Format::eR32G32B32Sfloat)
            .setOffset(0);

    // The second location is the vertex colors, in RGBA float4 format.
    // These appear in each element in memory after the float3 vertex
    // positions, so the offset is set accordingly.
//    m_vertices.vi_attrs[1].binding = VERTEX_BUFFER_BIND_ID;
//    m_vertices.vi_attrs[1].location = 1;
//    m_vertices.vi_attrs[1].format = VK_FORMAT_R32G32B32A32_SFLOAT; //float4
//    m_vertices.vi_attrs[1].offset = sizeof(float) * 3;

    m_vertices.vi_attrs[1].setBinding(VERTEX_BUFFER_BIND_ID)
            .setLocation(1)
            .setFormat(Format::eR32G32B32A32Sfloat)
            .setOffset(sizeof(float)*3);
}

///////////////////////////////////////////////////////////////////////////////

void VkSample::InitLayouts()
{
//    VkResult ret = VK_SUCCESS;
    // This sample has no bindings, so the layout is empty.
//    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {};
//    descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
//    descriptorSetLayoutCreateInfo.pNext = nullptr;
//    descriptorSetLayoutCreateInfo.bindingCount = 0;
//    descriptorSetLayoutCreateInfo.pBindings = nullptr;

    DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo;

//    ret = vkCreateDescriptorSetLayout(m_device, &descriptorSetLayoutCreateInfo, nullptr, &m_descriptorLayout);
//    VK_CHECK(!ret);
    descriptorLayout = device.createDescriptorSetLayout(descriptorSetLayoutCreateInfo);

    // Our pipeline layout simply points to the empty descriptor layout.
//    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
//    pipelineLayoutCreateInfo.sType              = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
//    pipelineLayoutCreateInfo.pNext              = nullptr;
//    pipelineLayoutCreateInfo.setLayoutCount     = 1;
//    pipelineLayoutCreateInfo.pSetLayouts        = &m_descriptorLayout;
//    ret = vkCreatePipelineLayout(m_device, &pipelineLayoutCreateInfo, nullptr, &m_pipelineLayout);
//    VK_CHECK(!ret);

    PipelineLayoutCreateInfo pipelineLayoutCreateInfo;
    pipelineLayoutCreateInfo.setPSetLayouts(&descriptorLayout).setSetLayoutCount(1);
    pipelineLayout = device.createPipelineLayout(pipelineLayoutCreateInfo);
}

///////////////////////////////////////////////////////////////////////////////

void VkSample::InitRenderPass()
{
    // The renderpass defines the attachments to the framebuffer object that gets
    // used in the pipeline. We have two attachments, the colour buffer, and the
    // depth buffer. The operations and layouts are set to defaults for this type
    // of attachment.
//    VkAttachmentDescription attachmentDescriptions[2] = {};
//    attachmentDescriptions[0].flags = 0;
//    attachmentDescriptions[0].format = m_surfaceFormat.format;
//    attachmentDescriptions[0].samples = VK_SAMPLE_COUNT_1_BIT;
//    attachmentDescriptions[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
//    attachmentDescriptions[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
//    attachmentDescriptions[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
//    attachmentDescriptions[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
//    attachmentDescriptions[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
//    attachmentDescriptions[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

//    attachmentDescriptions[1].flags = 0;
//    attachmentDescriptions[1].format = m_depthBuffers[0].format;
//    attachmentDescriptions[1].samples = VK_SAMPLE_COUNT_1_BIT;
//    attachmentDescriptions[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
//    attachmentDescriptions[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
//    attachmentDescriptions[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
//    attachmentDescriptions[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
//    attachmentDescriptions[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
//    attachmentDescriptions[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    AttachmentDescription attachDesc[2];
    attachDesc[0].setFormat(surfaceFormat.format).setSamples(SampleCountFlagBits::e1)
            .setLoadOp(AttachmentLoadOp::eClear).setStoreOp(AttachmentStoreOp::eStore)
            .setStencilLoadOp(AttachmentLoadOp::eDontCare).setStencilStoreOp(AttachmentStoreOp::eDontCare)
            .setInitialLayout(ImageLayout::eColorAttachmentOptimal).setFinalLayout(ImageLayout::eColorAttachmentOptimal);

    attachDesc[1].setFormat(m_depthBuffers[0].format).setSamples(SampleCountFlagBits::e1)
            .setLoadOp(AttachmentLoadOp::eClear).setStoreOp(AttachmentStoreOp::eDontCare)
            .setStencilLoadOp(AttachmentLoadOp::eDontCare).setStencilStoreOp(AttachmentStoreOp::eDontCare)
            .setInitialLayout(ImageLayout::eDepthStencilAttachmentOptimal).setFinalLayout(ImageLayout::eDepthStencilAttachmentOptimal);
    // We have references to the attachment offsets, stating the layout type.
//    VkAttachmentReference colorReference = {};
//    colorReference.attachment = 0;
//    colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

//    VkAttachmentReference depthReference = {};
//    depthReference.attachment = 1;
//    depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    AttachmentReference clrRef(0,ImageLayout::eColorAttachmentOptimal);
    AttachmentReference depthRef(1,ImageLayout::eDepthStencilAttachmentOptimal);

    // There can be multiple subpasses in a renderpass, but this example has only one.
    // We set the color and depth references at the grahics bind point in the pipeline.
//    VkSubpassDescription subpassDescription = {};
//    subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
//    subpassDescription.flags = 0;
//    subpassDescription.inputAttachmentCount = 0;
//    subpassDescription.pInputAttachments = nullptr;
//    subpassDescription.colorAttachmentCount = 1;
//    subpassDescription.pColorAttachments = &colorReference;
//    subpassDescription.pResolveAttachments = nullptr;
//    subpassDescription.pDepthStencilAttachment = &depthReference;
//    subpassDescription.preserveAttachmentCount = 0;
//    subpassDescription.pPreserveAttachments = nullptr;

    SubpassDescription subpassDesc;
    subpassDesc.setPipelineBindPoint(PipelineBindPoint::eGraphics)
            .setColorAttachmentCount(1).setPColorAttachments(&clrRef)
            .setPDepthStencilAttachment(&depthRef);

    // The renderpass itself is created with the number of subpasses, and the
    // list of attachments which those subpasses can reference.
//    VkRenderPassCreateInfo renderPassCreateInfo = {};
//    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
//    renderPassCreateInfo.pNext = nullptr;
//    renderPassCreateInfo.attachmentCount = 2;
//    renderPassCreateInfo.pAttachments = attachmentDescriptions;
//    renderPassCreateInfo.subpassCount = 1;
//    renderPassCreateInfo.pSubpasses = &subpassDescription;
//    renderPassCreateInfo.dependencyCount = 0;
//    renderPassCreateInfo.pDependencies = nullptr;

//    VkResult ret;
//    ret = vkCreateRenderPass(m_device, &renderPassCreateInfo, nullptr, &m_renderPass);
//    VK_CHECK(!ret);

    RenderPassCreateInfo createInfo;
    createInfo.setAttachmentCount(2).setPAttachments(attachDesc)
            .setSubpassCount(1).setPSubpasses(&subpassDesc);
    renderPass = device.createRenderPass(createInfo);
}

///////////////////////////////////////////////////////////////////////////////

void VkSample::InitPipeline()
{
    VkResult   err;

    // The pipeline contains all major state for rendering.

    // Our vertex input is a single vertex buffer, and its layout is defined
    // in our m_vertices object already. Use this when creating the pipeline.
//    VkPipelineVertexInputStateCreateInfo   vi = {};
//    vi = m_vertices.vi;

    PipelineVertexInputStateCreateInfo vi = m_vertices.vi;

    // Our vertex buffer describes a triangle list.
//    VkPipelineInputAssemblyStateCreateInfo ia = {};
//    ia.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
//    ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    PipelineInputAssemblyStateCreateInfo ia;
    ia.setTopology(PrimitiveTopology::eTriangleList);

    // State for rasterization, such as polygon fill mode is defined.
//    VkPipelineRasterizationStateCreateInfo rs = {};
//    rs.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
//    rs.polygonMode = VK_POLYGON_MODE_FILL;
//    rs.cullMode = VK_CULL_MODE_BACK_BIT;
//    rs.frontFace = VK_FRONT_FACE_CLOCKWISE;
//    rs.depthClampEnable = VK_FALSE;
//    rs.rasterizerDiscardEnable = VK_FALSE;
//    rs.depthBiasEnable = VK_FALSE;

    PipelineRasterizationStateCreateInfo rs;
    rs.setPolygonMode(PolygonMode::eFill)
            .setCullMode(CullModeFlagBits::eBack)
            .setFrontFace(FrontFace::eClockwise)
            .setDepthClampEnable(VK_FALSE)
            .setDepthBiasEnable(VK_FALSE)
            .setRasterizerDiscardEnable(VK_FALSE);

    // For this example we do not do blending, so it is disabled.
//    VkPipelineColorBlendAttachmentState att_state[1] = {};
//    att_state[0].colorWriteMask = 0xf;
//    att_state[0].blendEnable = VK_FALSE;

    PipelineColorBlendAttachmentState attachmentState[1];
    attachmentState[0].setColorWriteMask((ColorComponentFlagBits)0xf);
    attachmentState[0].setBlendEnable(VK_FALSE);

//    VkPipelineColorBlendStateCreateInfo    cb = {};
//    cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
//    cb.attachmentCount = 1;
//    cb.pAttachments = &att_state[0];

    PipelineColorBlendStateCreateInfo cb;
    cb.setAttachmentCount(1).setPAttachments(attachmentState);

    // We define a simple viewport and scissor. It does not change during rendering
    // in this sample.
//    VkPipelineViewportStateCreateInfo      vp = {};
//    vp.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
//    vp.viewportCount = 1;
//    vp.scissorCount = 1;

    Viewport viewPort(0,0,(float)m_width,(float)m_height,0.f,1.f);
    Rect2D scissor(Offset2D(0,0),Extent2D(m_width, m_height));
    PipelineViewportStateCreateInfo vp;
    vp.setScissorCount(1).setPScissors(&scissor)
            .setViewportCount(1).setPViewports(&viewPort);

//    VkViewport viewport = {};
//    viewport.height = (float) m_height;
//    viewport.width = (float) m_width;
//    viewport.minDepth = (float) 0.0f;
//    viewport.maxDepth = (float) 1.0f;
//    vp.pViewports = &viewport;

//    VkRect2D scissor = {};
//    scissor.extent.width = m_width;
//    scissor.extent.height = m_height;
//    scissor.offset.x = 0;
//    scissor.offset.y = 0;
//    vp.pScissors = &scissor;

    // Standard depth and stencil state is defined
//    VkPipelineDepthStencilStateCreateInfo  ds = {};
//    ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
//    ds.depthTestEnable = VK_TRUE;
//    ds.depthWriteEnable = VK_TRUE;
//    ds.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
//    ds.depthBoundsTestEnable = VK_FALSE;
//    ds.back.failOp = VK_STENCIL_OP_KEEP;
//    ds.back.passOp = VK_STENCIL_OP_KEEP;
//    ds.back.compareOp = VK_COMPARE_OP_ALWAYS;
//    ds.stencilTestEnable = VK_FALSE;
//    ds.front = ds.back;

    PipelineDepthStencilStateCreateInfo ds;
    ds.setDepthTestEnable(VK_TRUE).setDepthWriteEnable(VK_TRUE).setDepthCompareOp(CompareOp::eLessOrEqual)
            .setDepthBoundsTestEnable(VK_FALSE).setBack(StencilOpState(StencilOp::eKeep,StencilOp::eKeep,StencilOp::eKeep,CompareOp::eAlways))
            .setStencilTestEnable(VK_FALSE).setFront(StencilOpState(StencilOp::eKeep,StencilOp::eKeep,StencilOp::eKeep,CompareOp::eAlways));

    // We do not use multisample
//    VkPipelineMultisampleStateCreateInfo   ms = {};
//    ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
//    ms.pSampleMask = nullptr;
//    ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    PipelineMultisampleStateCreateInfo ms;
    ms.setRasterizationSamples(SampleCountFlagBits::e1);

    // We define two shader stages: our vertex and fragment shader.
    // they are embedded as SPIR-V into a header file for ease of deployment.
//    VkPipelineShaderStageCreateInfo shaderStages[2] = {};
//    shaderStages[0].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
//    shaderStages[0].stage  = VK_SHADER_STAGE_VERTEX_BIT;
//    shaderStages[0].module = CreateShaderModule(v);
//    shaderStages[0].pName  = "main";
//    shaderStages[1].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
//    shaderStages[1].stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
//    shaderStages[1].module = CreateShaderModule( (const uint32_t*)&shader_tri_frag[0], shader_tri_frag_size);
//    shaderStages[1].pName  = "main";

    PipelineShaderStageCreateInfo stages[2];
    stages[0].setStage(ShaderStageFlagBits::eVertex)
            .setModule(CreateShaderModule((const uint32_t*)&shader_tri_vert[0], shader_tri_vert_size))
            .setPName("main");
    stages[1].setStage(ShaderStageFlagBits::eFragment)
            .setModule(CreateShaderModule((const uint32_t*)&shader_tri_frag[0], shader_tri_frag_size))
            .setPName("main");

    // Pipelines are allocated from pipeline caches.
//    VkPipelineCacheCreateInfo pipelineCache = {};
//    pipelineCache.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
//    pipelineCache.pNext = nullptr;
//    pipelineCache.flags = 0;

    PipelineCacheCreateInfo pipelineCacheInfo;
    PipelineCache pipelineCache = device.createPipelineCache(pipelineCacheInfo);

//    VkPipelineCache piplineCache;
//    err = vkCreatePipelineCache(m_device, &pipelineCache, nullptr, &piplineCache);
//    VK_CHECK(!err);

    // Out graphics pipeline records all state information, including our renderpass
    // and pipeline layout. We do not have any dynamic state in this example.
//    VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
//    pipelineCreateInfo.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
//    pipelineCreateInfo.layout              = m_pipelineLayout;
//    pipelineCreateInfo.pVertexInputState   = &vi;
//    pipelineCreateInfo.pInputAssemblyState = &ia;
//    pipelineCreateInfo.pRasterizationState = &rs;
//    pipelineCreateInfo.pColorBlendState    = &cb;
//    pipelineCreateInfo.pMultisampleState   = &ms;
//    pipelineCreateInfo.pViewportState      = &vp;
//    pipelineCreateInfo.pDepthStencilState  = &ds;
//    pipelineCreateInfo.pStages             = &shaderStages[0];
//    pipelineCreateInfo.renderPass          = m_renderPass;
//    pipelineCreateInfo.pDynamicState       = nullptr;
//    pipelineCreateInfo.stageCount          = 2; //vertex and fragment

//    err = vkCreateGraphicsPipelines(m_device, piplineCache, 1, &pipelineCreateInfo, nullptr, &m_pipeline);
//    VK_CHECK(!err);

    GraphicsPipelineCreateInfo pipeInfo;
    pipeInfo.setLayout(pipelineLayout).setPVertexInputState(&vi).setPInputAssemblyState(&ia)
            .setPRasterizationState(&rs).setPColorBlendState(&cb).setPMultisampleState(&ms)
            .setPViewportState(&vp).setPDepthStencilState(&ds)
            .setRenderPass(renderPass).setStageCount(2).setPStages(stages);
    pipeline = device.createGraphicsPipeline(pipelineCache,pipeInfo);

    // We can destroy the cache now as we do not need it. The shader modules also
    // can be destroyed after the pipeline is created.
//    vkDestroyPipelineCache(m_device, piplineCache, nullptr);
    device.destroyPipelineCache(pipelineCache);

//    vkDestroyShaderModule(m_device, shaderStages[0].module, nullptr);
//    vkDestroyShaderModule(m_device, shaderStages[1].module, nullptr);
    device.destroyShaderModule(stages[0].module);
    device.destroyShaderModule(stages[1].module);
}
///////////////////////////////////////////////////////////////////////////////

ShaderModule VkSample::CreateShaderModule(const uint32_t* code, uint32_t size)
{
//    VkShaderModule module;
//    VkResult  err;
//
//    // Creating a shader is very simple once it's in memory as compiled SPIR-V.
//    VkShaderModuleCreateInfo moduleCreateInfo = {};
//    moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
//    moduleCreateInfo.pNext = nullptr;
//    moduleCreateInfo.codeSize = size;
//    moduleCreateInfo.pCode = code;
//    moduleCreateInfo.flags = 0;
//    err = vkCreateShaderModule(m_device, &moduleCreateInfo, nullptr, &module);
//    VK_CHECK(!err);

    ShaderModuleCreateInfo createInfo;
    createInfo.setCodeSize(size).setPCode(code);
    return device.createShaderModule(createInfo);
}
///////////////////////////////////////////////////////////////////////////////

void VkSample::InitFrameBuffers()
{

    //The framebuffer objects reference the renderpass, and allow
    // the references defined in that renderpass to now attach to views.
    // The views in this example are the colour view, which is our swapchain image,
    // and the depth buffer created manually earlier.
//    VkImageView attachments[2] = {};
//    VkFramebufferCreateInfo framebufferCreateInfo = {};
//    framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
//    framebufferCreateInfo.pNext = nullptr;
//    framebufferCreateInfo.renderPass = m_renderPass;
//    framebufferCreateInfo.attachmentCount = 2;
//    framebufferCreateInfo.pAttachments = attachments;
//    framebufferCreateInfo.width  = m_width;
//    framebufferCreateInfo.height = m_height;
//    framebufferCreateInfo.layers = 1;

    ImageView attachments[2] = {};
    FramebufferCreateInfo fboInfo;
    fboInfo.setRenderPass(renderPass)
            .setAttachmentCount(2).setPAttachments(attachments)
            .setWidth(m_width).setHeight(m_height).setLayers(1);

//    VkResult ret;

//    m_frameBuffers = new VkFramebuffer[m_swapchainImageCount];
    framebuffers.resize(m_swapchainImageCount);
    // Reusing the framebufferCreateInfo to create m_swapchainImageCount framebuffers,
    // only the attachments to the relevent image views change each time.
    for (uint32_t i = 0; i < m_swapchainImageCount; i++) {
        attachments[0] = m_swapchainBuffers[i].view;
        attachments[1] = m_depthBuffers[i].view;

//        ret = vkCreateFramebuffer(m_device, &framebufferCreateInfo, nullptr, &m_frameBuffers[i]);
//        VK_CHECK(!ret);
        framebuffers[i] = device.createFramebuffer(fboInfo);
    }
}

///////////////////////////////////////////////////////////////////////////////

void VkSample::InitSync()
{
//    VkResult ret = VK_SUCCESS;
    // For synchronization, we have semaphores for rendering and backbuffer signalling.
//    VkSemaphoreCreateInfo semaphoreCreateInfo = {};
//    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
//    semaphoreCreateInfo.pNext = nullptr;
//    semaphoreCreateInfo.flags = 0;
//    ret = vkCreateSemaphore(m_device, &semaphoreCreateInfo, nullptr, &m_backBufferSemaphore);
//    VK_CHECK(!ret);

//    ret = vkCreateSemaphore(m_device, &semaphoreCreateInfo, nullptr, &m_renderCompleteSemaphore);
//    VK_CHECK(!ret);
    SemaphoreCreateInfo info;
    backBufferSemaphore = device.createSemaphore(info);
    renderCompleteSemaphore = device.createSemaphore(info);
}

///////////////////////////////////////////////////////////////////////////////

bool VkSample::GetMemoryTypeFromProperties( uint32_t typeBits, MemoryPropertyFlags requirements_mask, uint32_t* typeIndex)
{
    VK_CHECK(typeIndex != nullptr);
    // Search memtypes to find first index with those properties
    for (uint32_t i = 0; i < 32; i++) {
        if ((typeBits & 1) == 1) {
            // Type is available, does it match user properties?
            if ((physicalDeviceMemoryProps.memoryTypes[i].propertyFlags &
                 requirements_mask) == requirements_mask) {
                *typeIndex = i;
                return true;
            }
        }
        typeBits >>= 1;
    }
    // No memory types matched, return failure
    return false;
}

///////////////////////////////////////////////////////////////////////////////

void VkSample::SetNextBackBuffer()
{
//    VkResult ret = VK_SUCCESS;

    // Get the next image to render to, then queue a wait until the image is ready
//    ret  = vkAcquireNextImageKHR(m_device, m_swapchain, UINT64_MAX, m_backBufferSemaphore, VK_NULL_HANDLE, &m_swapchainCurrentIdx);
//    if (ret == VK_ERROR_OUT_OF_DATE_KHR) {
//        LOGW("VK_ERROR_OUT_OF_DATE_KHR not handled in sample");
//    } else if (ret == VK_SUBOPTIMAL_KHR) {
//        LOGW("VK_SUBOPTIMAL_KHR not handled in sample");
//    }
//    VK_CHECK(!ret);

    Result ret = device.acquireNextImageKHR(swapChain, UINT64_MAX, backBufferSemaphore, Fence(), &m_swapchainCurrentIdx);
}

///////////////////////////////////////////////////////////////////////////////

void VkSample::PresentBackBuffer()
{
//    VkResult ret = VK_SUCCESS;

    // Use WSI to present. The semaphore chain used to signal rendering
    // completion allows the operation to wait before the present is
    // completed.
//    VkPresentInfoKHR presentInfo = {};
//    presentInfo.sType          = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
//    presentInfo.swapchainCount = 1;
//    presentInfo.pSwapchains    = &m_swapchain;
//    presentInfo.pImageIndices  = &m_swapchainCurrentIdx;
//    presentInfo.waitSemaphoreCount = 1;
//    presentInfo.pWaitSemaphores= &m_renderCompleteSemaphore;

//    ret = vkQueuePresentKHR(m_queue, &presentInfo);
//    VK_CHECK(!ret);

    PresentInfoKHR presentInfoKHR(1,&renderCompleteSemaphore,1,&swapChain,&m_swapchainCurrentIdx);
    queue.presentKHR(presentInfoKHR);

    // Obtain the back buffer for the next frame.
    SetNextBackBuffer();
}

///////////////////////////////////////////////////////////////////////////////

bool VkSample::TearDown()
{
    if (!m_initBegun) 
    {
        return false;
    }

    // Destroy all resources for framebuffers, swapchain images, and depth buffers
    for (uint32_t i = 0; i < m_swapchainImageCount; i++) {
//        vkDestroyFramebuffer(m_device, m_frameBuffers[i], nullptr);
        device.destroyFramebuffer(framebuffers[i]);
//        vkFreeCommandBuffers(m_device, m_commandPool, 1, &m_swapchainBuffers[i].cmdBuffer);
        device.freeCommandBuffers(commandPool, 1, &m_swapchainBuffers[i].cmdBuffer);
//        vkDestroyImageView(m_device, m_swapchainBuffers[i].view, nullptr);
        device.destroyImageView(m_swapchainBuffers[i].view);
//        vkDestroyImage(m_device, m_swapchainBuffers[i].image, nullptr);
        device.destroyImage(m_swapchainBuffers[i].image);
//        vkDestroyImageView(m_device, m_depthBuffers[i].view, nullptr);
        device.destroyImageView(m_depthBuffers[i].view);
//        vkDestroyImage(m_device, m_depthBuffers[i].image, nullptr);
        device.destroyImage(m_depthBuffers[i].image);
//        vkFreeMemory(m_device, m_depthBuffers[i].mem, nullptr);
        device.freeMemory(m_depthBuffers[i].mem);
    }

//    delete [] m_frameBuffers;
//    delete [] m_swapchainBuffers;
    delete [] m_depthBuffers;

    // Destroy pipeline resources
//    vkDestroyPipeline(m_device, m_pipeline, nullptr);
    device.destroyPipeline(pipeline);
//    vkDestroyRenderPass(m_device, m_renderPass, nullptr);
    device.destroyRenderPass(renderPass);
//    vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);
    device.destroyPipelineLayout(pipelineLayout);
//    vkDestroyDescriptorSetLayout(m_device, m_descriptorLayout, nullptr);
    device.destroyDescriptorSetLayout(descriptorLayout);

    // Destroy pools
//    vkDestroyCommandPool(m_device, m_commandPool, nullptr);
    device.destroyCommandPool(commandPool);

    // Destroy sync
//    vkDestroySemaphore( m_device, m_backBufferSemaphore, nullptr);
    device.destroySemaphore(backBufferSemaphore);
//    vkDestroySemaphore( m_device, m_renderCompleteSemaphore, nullptr);
    device.destroySemaphore(renderCompleteSemaphore);

    // Destroy vertices
//    vkDestroyBuffer(m_device, m_vertices.buf, nullptr);
    device.destroyBuffer(m_vertices.buf);
//    vkFreeMemory(m_device, m_vertices.mem, nullptr);
    device.freeMemory(m_vertices.mem);

    // Destroy the swapchain
//    vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
    device.destroySwapchainKHR(swapChain);

    // Destroy the device, surface and instance
//    vkDestroyDevice(m_device, nullptr);
    device.destroy();

//    vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
    instance.destroySurfaceKHR(surface);
//    vkDestroyInstance(m_instance, nullptr);
    instance.destroy();

//    delete [] m_pPhysicalDevices;

    m_initialized = false;
    m_initBegun = false;
    
    return true;
}

///////////////////////////////////////////////////////////////////////////////

void VkSample::BuildCmdBuffer()
{
    // For the triangle sample, we pre-record our command buffer, as it is static.
    // We have a buffer per swap chain image, so loop over the creation process.
    for (uint32_t i = 0; i < m_swapchainImageCount; i++) {
        CommandBuffer &cmdBuffer = m_swapchainBuffers[i].cmdBuffer;

        // vkBeginCommandBuffer should reset the command buffer, but Reset can be called
        // to make it more explicit.
//        VkResult err;
//        err = vkResetCommandBuffer(cmdBuffer, 0);
//        VK_CHECK(!err);

        cmdBuffer.reset(CommandBufferResetFlagBits(0));

//        VkCommandBufferInheritanceInfo cmd_buf_hinfo = {};
//        cmd_buf_hinfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
//        cmd_buf_hinfo.pNext = nullptr;
//        cmd_buf_hinfo.renderPass = VK_NULL_HANDLE;
//        cmd_buf_hinfo.subpass = 0;
//        cmd_buf_hinfo.framebuffer = VK_NULL_HANDLE;
//        cmd_buf_hinfo.occlusionQueryEnable = VK_FALSE;
//        cmd_buf_hinfo.queryFlags = 0;
//        cmd_buf_hinfo.pipelineStatistics = 0;

//        VkCommandBufferBeginInfo cmd_buf_info = {};
//        cmd_buf_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
//        cmd_buf_info.pNext = nullptr;
//        cmd_buf_info.flags = 0;
//        cmd_buf_info.pInheritanceInfo = &cmd_buf_hinfo;

        CommandBufferInheritanceInfo inheritanceInfo;
        CommandBufferBeginInfo cmdBeginInfo(CommandBufferUsageFlagBits::eSimultaneousUse);
        //cmdBeginInfo.setPInheritanceInfo(&inheritanceInfo);

        // By calling vkBeginCommandBuffer, cmdBuffer is put into the recording state.
//        err = vkBeginCommandBuffer(cmdBuffer, &cmd_buf_info);
//        VK_CHECK(!err);
        cmdBuffer.begin(cmdBeginInfo);


        // Before we can use the back buffer from the swapchain, we must change the
        // image layout from the PRESENT mode to the COLOR_ATTACHMENT mode.
        // PRESENT mode is optimal for sending to the screen for users to see, so the
        // image will be set back to that mode after we have completed rendering.
//        VkImageMemoryBarrier preRenderBarrier = {};
//        preRenderBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
//        preRenderBarrier.pNext = nullptr;
//        preRenderBarrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
//        preRenderBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
//        preRenderBarrier.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
//        preRenderBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
//        preRenderBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
//        preRenderBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
//        preRenderBarrier.image = m_swapchainBuffers[i].image;
//        preRenderBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//        preRenderBarrier.subresourceRange.baseArrayLayer = 0;
//        preRenderBarrier.subresourceRange.baseMipLevel = 1;
//        preRenderBarrier.subresourceRange.layerCount = 0;
//        preRenderBarrier.subresourceRange.levelCount = 1;

        ImageMemoryBarrier preRenderBarrier(AccessFlagBits::eMemoryRead,AccessFlagBits::eColorAttachmentWrite,
                                      ImageLayout::ePresentSrcKHR,ImageLayout::eColorAttachmentOptimal,
                                      VK_QUEUE_FAMILY_IGNORED,VK_QUEUE_FAMILY_IGNORED,
                                      m_swapchainBuffers[i].image,
                                        ImageSubresourceRange(ImageAspectFlagBits::eColor,1,1,0,0));

        // Thie PipelineBarrier function can operate on memoryBarriers,
        // bufferMemory and imageMemory buffers. We only provide a single
        // imageMemoryBarrier.
//        vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
//                             VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
//                             0, 0, nullptr, 0, nullptr, 1, &preRenderBarrier);
        cmdBuffer.pipelineBarrier(PipelineStageFlagBits::eColorAttachmentOutput,PipelineStageFlagBits::eColorAttachmentOutput,
                                  DependencyFlags(),0,nullptr,0,nullptr,1,&preRenderBarrier);
                             
        // When starting the render pass, we can set clear values.
//        VkClearValue clear_values[2] = {};
//        clear_values[0].color.float32[0] = 0.3f;
//        clear_values[0].color.float32[1] = 0.3f;
//        clear_values[0].color.float32[2] = 0.3f;
//        clear_values[0].color.float32[3] = 1.0f;
//        clear_values[1].depthStencil.depth = 1.0f;
//        clear_values[1].depthStencil.stencil = 0;

//        VkRenderPassBeginInfo rp_begin = {};
//        rp_begin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
//        rp_begin.pNext = nullptr;
//        rp_begin.renderPass = m_renderPass;
//        rp_begin.framebuffer = m_frameBuffers[i];
//        rp_begin.renderArea.offset.x = 0;
//        rp_begin.renderArea.offset.y = 0;
//        rp_begin.renderArea.extent.width = m_width;
//        rp_begin.renderArea.extent.height = m_height;
//        rp_begin.clearValueCount = 2;
//        rp_begin.pClearValues = clear_values;

//        vkCmdBeginRenderPass(cmdBuffer, &rp_begin, VK_SUBPASS_CONTENTS_INLINE);

        ClearValue clearValues[2];
        clearValues[0].color.float32[0] = 0.3f;
        clearValues[0].color.float32[1] = 0.3f;
        clearValues[0].color.float32[2] = 0.3f;
        clearValues[0].color.float32[3] = 0.3f;
        clearValues[1].depthStencil.depth = 1.0f;
        clearValues[1].depthStencil.stencil = 1;

        RenderPassBeginInfo beginInfo(renderPass, framebuffers[i],
                                      Rect2D(Offset2D(), Extent2D(m_width,m_height)), 2, clearValues);
        cmdBuffer.beginRenderPass(beginInfo, SubpassContents::eInline);

        // Set our pipeline. This holds all major state
        // the pipeline defines, for example, that the vertex buffer is a triangle list.
//        vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
        cmdBuffer.bindPipeline(PipelineBindPoint::eGraphics,pipeline);

        // Bind our vertex buffer, with a 0 offset.
        VkDeviceSize offsets[1] = {0};
//        vkCmdBindVertexBuffers(cmdBuffer, VERTEX_BUFFER_BIND_ID, 1, &m_vertices.buf, offsets);
        cmdBuffer.bindVertexBuffers(VERTEX_BUFFER_BIND_ID, 1, &m_vertices.buf, offsets);

        // Issue a draw command, with our 3 vertices.
//        vkCmdDraw(cmdBuffer, 3, 1, 0, 0);
        cmdBuffer.draw(3,1,0,0);

        // Now our render pass has ended.
//        vkCmdEndRenderPass(cmdBuffer);
        cmdBuffer.endRenderPass();

        // As stated earlier, now transition the swapchain image to the PRESENT mode.
//        VkImageMemoryBarrier prePresentBarrier = {};
//        prePresentBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
//        prePresentBarrier.pNext = nullptr;
//        prePresentBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
//        prePresentBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
//        prePresentBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
//        prePresentBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
//        prePresentBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
//        prePresentBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
//        prePresentBarrier.image = m_swapchainBuffers[i].image;
//        prePresentBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//        prePresentBarrier.subresourceRange.baseArrayLayer = 0;
//        prePresentBarrier.subresourceRange.baseMipLevel = 1;
//        prePresentBarrier.subresourceRange.layerCount = 0;

//        vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
//                             VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
//                             0, 0, nullptr, 0, nullptr, 1, &prePresentBarrier);

        ImageMemoryBarrier prePresentBarrier(AccessFlagBits::eColorAttachmentWrite,AccessFlagBits::eMemoryRead,
                                            ImageLayout::eColorAttachmentOptimal,ImageLayout::ePresentSrcKHR,
                                            VK_QUEUE_FAMILY_IGNORED,VK_QUEUE_FAMILY_IGNORED,
                                            m_swapchainBuffers[i].image,
                                            ImageSubresourceRange(ImageAspectFlagBits::eColor,1,1,0,0));

        cmdBuffer.pipelineBarrier(PipelineStageFlagBits::eColorAttachmentOutput,PipelineStageFlagBits::eBottomOfPipe,
                                  DependencyFlags(),0,nullptr,0,nullptr,1,&prePresentBarrier);

        // By ending the command buffer, it is put out of record mode.
//        err = vkEndCommandBuffer(cmdBuffer);
//        VK_CHECK(!err);
        cmdBuffer.end();
    }
}

///////////////////////////////////////////////////////////////////////////////

void VkSample::DrawFrame()
{
    if (!m_initialized)
    {
        return;
    }

//    VkFence nullFence = VK_NULL_HANDLE;

//    VkSubmitInfo submitInfo = {};
//    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
//    submitInfo.pNext = nullptr;
//    submitInfo.waitSemaphoreCount = 1;
//    submitInfo.pWaitSemaphores = &m_backBufferSemaphore;
//    submitInfo.pWaitDstStageMask = nullptr;
//    submitInfo.commandBufferCount = 1;
//    submitInfo.pCommandBuffers = &m_swapchainBuffers[m_swapchainCurrentIdx].cmdBuffer;
//    submitInfo.signalSemaphoreCount = 1;
//    submitInfo.pSignalSemaphores = &m_renderCompleteSemaphore;

//    VkResult err;

//    err = vkQueueSubmit(m_queue, 1, &submitInfo,  VK_NULL_HANDLE);
//    VK_CHECK(!err);

    SubmitInfo info(1, &backBufferSemaphore, nullptr,
                    1, &m_swapchainBuffers[m_swapchainCurrentIdx].cmdBuffer,
                    1, &renderCompleteSemaphore);
    queue.submit(1, &info, Fence());


    PresentBackBuffer();


    // Calculate FPS below, displaying every 30 frames
    m_frameTimeEnd = GetTime();
    if (m_frameIdx % 30 == 0)
    {
        uint64_t frameDelta = m_frameTimeEnd - m_frameTimeBegin;
        double fps = 1000000000.0/((double)frameDelta);

        LOGI("FPS: %f", fps);
    }
    m_frameIdx++;
    m_frameTimeBegin = GetTime();
}

///////////////////////////////////////////////////////////////////////////////

uint64_t VkSample::GetTime()
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (uint64_t) now.tv_sec*1000000000LL + now.tv_nsec;
}

///////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////
// Begin Android Glue entry point

/**
 * Shared state for our app.
 */
struct engine {
    struct android_app* app;
    int animating;
    VkSample sample;
};

/**
 * Process the next main command.
 */
static void engine_handle_cmd(struct android_app* app, int32_t cmd) {
    struct engine* engine = (struct engine*)app->userData;
    switch (cmd) {
        case APP_CMD_SAVE_STATE:
            // Teardown, and recreate each time
            engine->animating = 0;
            engine->sample.TearDown();
            break;
        case APP_CMD_INIT_WINDOW:
        {
            if(!engine->sample.Initialize(engine->app->window))
            {
                LOGE("VkSample::Initialize Error");
                engine->sample.TearDown();
            }
            else
            {
                LOGI("VkSample::Initialize Success");
            }
            engine->animating = 1;
            break;}
        case APP_CMD_TERM_WINDOW:
            engine->sample.TearDown();
            break;
        case APP_CMD_GAINED_FOCUS:
            engine->animating = 1;
            break;
        case APP_CMD_LOST_FOCUS:
            engine->animating = 0;
            break;
    }
}

/**
 * This is the main entry point of a native application that is using
 * android_native_app_glue.  It runs in its own thread, with its own
 * event loop for receiving input events and doing other things.
 */
void android_main(struct android_app* state) {
    struct engine engine;

    // Make sure glue isn't stripped.
    app_dummy();

    memset(&engine, 0, sizeof(engine));
    state->userData = &engine;
    state->onAppCmd = engine_handle_cmd;
    engine.app = state;

    // loop waiting for stuff to do.
    while (1) {
        int ident;
        int events;
        struct android_poll_source* source;

        // If not animating, we will block forever waiting for events.
        // If animating, we loop until all events are read, then continue
        // to draw the next frame of animation.
        while ((ident=ALooper_pollAll(engine.animating ? 0 : -1, nullptr, &events,
                                      (void**)&source)) >= 0) {

            // Process this event.
            if (source != nullptr) {
                source->process(state, source);
            }

            // Check if we are exiting.
            if (state->destroyRequested != 0) {
                engine.sample.TearDown();
                return;
            }
        }

        if (engine.animating && engine.sample.IsInitialized())
        {
            engine.sample.DrawFrame();
        }
    }
}
//END Android Glue
///////////////////////////////////////////////////////////////////////////////
