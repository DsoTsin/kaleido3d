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
    VkBool32 surfaceExtFound = 0;
    VkBool32 platformSurfaceExtFound = 0;
    // We require two extensions, VK_KHR_surface and VK_KHR_android_surface. If they are found,
    // add them to the extensionNames list that we'll use to initialize our instance with later.
    uint32_t enabledExtensionCount = 0;
    const char* extensionNames[16];

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
    vk::ApplicationInfo appInfo(SAMPLE_NAME, 0, "VkSample", 1, VK_MAKE_VERSION(1,0,0));

    // Creation information for the instance points to details about
    // the application, and also the list of extensions to enable.
    vk::InstanceCreateInfo instCreateInfo;
    instCreateInfo.setPApplicationInfo(&appInfo)
            .setEnabledExtensionCount(enabledExtensionCount)
            .setPpEnabledExtensionNames(extensionNames);


    // The main Vulkan instance is created with the creation infos above.
    // We do not specify a custom memory allocator for instance creation.
    instance = vk::createInstance(instCreateInfo);

    return true;
}
///////////////////////////////////////////////////////////////////////////////

bool VkSample::GetPhysicalDevices()
{
    std::vector<vk::PhysicalDevice> devices = instance.enumeratePhysicalDevices();
    physicalDevice = devices[0];
    physicalDeviceProps = physicalDevice.getProperties();
    LOGI("Vulkan Device: %s", physicalDeviceProps.deviceName);
    physicalDeviceMemoryProps = physicalDevice.getMemoryProperties();
    return true;
}

///////////////////////////////////////////////////////////////////////////////

void VkSample::InitDevice()
{
    std::vector<vk::ExtensionProperties> extProps = physicalDevice.enumerateDeviceExtensionProperties();
    VkBool32 swapchainExtFound = 0;
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
    vector<QueueFamilyProperties> queueFamilyProps = physicalDevice.getQueueFamilyProperties();

    // We query each queue family in turn for the ability to support the android surface
    // that was created earlier. We need the device to be able to present its images to
    // this surface, so it is important to test for this.
    VkBool32* supportsPresent = new VkBool32[queueFamilyProps.size()];
    for (uint32_t i = 0; i < queueFamilyProps.size(); i++) {
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

    if (queueIndex == UINT32_MAX) {
        VK_CHECK("Could not obtain a queue family for both graphics and presentation." && 0);
    }

    // We have identified a queue family which both supports our android surface,
    // and can be used for graphics operations.
    m_queueFamilyIndex = queueIndex;


    // As we create the device, we state we will be creating a queue of the
    // family type required. 1.0 is the highest priority and we use that.
    float queuePriorities[1] = { 1.0 };

    vk::DeviceQueueCreateInfo queueCreateInfo;
    queueCreateInfo.setQueueCount(1).setQueueFamilyIndex(m_queueFamilyIndex).setPQueuePriorities(queuePriorities);

    // Now we pass the queue create info, as well as our requested extensions,
    // into our DeviceCreateInfo structure.
    vk::DeviceCreateInfo deviceCreateInfo;
    deviceCreateInfo.setQueueCreateInfoCount(1).setPQueueCreateInfos(&queueCreateInfo)
            .setEnabledExtensionCount(enabledExtensionCount).setPpEnabledExtensionNames(extensionNames);


    // Create the device.
    device = physicalDevice.createDevice(deviceCreateInfo);

    // Obtain the device queue that we requested.
    queue = device.getQueue(m_queueFamilyIndex,0);
}

///////////////////////////////////////////////////////////////////////////////

void VkSample::InitSurface()
{
    vk::AndroidSurfaceCreateInfoKHR surfaceCreateInfoKHR;
    surfaceCreateInfoKHR.setWindow(m_androidWindow);
    surface = instance.createAndroidSurfaceKHR(surfaceCreateInfoKHR);
}

///////////////////////////////////////////////////////////////////////////////

void VkSample::InitSwapchain()
{
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

    // Now we obtain the surface capabilities, which contains details such as width and height.
    SurfaceCapabilitiesKHR surfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface);

    m_width = surfaceCapabilities.currentExtent.width;
    m_height = surfaceCapabilities.currentExtent.height;

    // Now that we have selected formats and obtained ideal surface dimensions,
    // we create the swapchain. We use FIFO mode, which is always present. This
    // mode has a queue of images internally, that will be presented to the screen.
    // The swapchain will be created and expose the number of images created
    // in the queue, which will be at least the number specified in minImageCount.
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

    // Now we can retrieve these images, as to use them in rendering as our framebuffers.
    vector<Image> swapChainImages = device.getSwapchainImagesKHR(swapChain);
    m_swapchainImageCount = swapChainImages.size();
    LOGI("Swapchain Image Count: %d\n", m_swapchainImageCount);
    // We prepare our own representation of the swapchain buffers, for keeping track
    // of resources during rendering.
    m_swapchainBuffers = new SwapchainBuffer[m_swapchainImageCount];
    VK_CHECK(m_swapchainBuffers);


    // From the images obtained from the swapchain, we create image views.
    // This gives us context into the image.
    ImageViewCreateInfo viewCreateInfo;
    viewCreateInfo.setFormat(surfaceFormat.format).setSubresourceRange(ImageSubresourceRange(ImageAspectFlagBits::eColor,0,1,0,1))
            .setComponents(ComponentMapping(ComponentSwizzle::eR,ComponentSwizzle::eG,ComponentSwizzle::eB,ComponentSwizzle::eA))
            .setViewType(ImageViewType::e2D);

    for (uint32_t i = 0; i < m_swapchainImageCount; i++) {
        // We create an Imageview for each swapchain image, and track
        // the view and image in our swapchainBuffers object.
        m_swapchainBuffers[i].image = swapChainImages[i];
        viewCreateInfo.image = swapChainImages[i];
        m_swapchainBuffers[i].view = device.createImageView(viewCreateInfo);
    }

    // Now we create depth buffers for our swapchain images, which form part of
    // our framebuffers later.
    m_depthBuffers = new DepthBuffer[m_swapchainImageCount];
    for (int i = 0; i < m_swapchainImageCount; i++) {
        const Format depthFmt = Format::eD16Unorm;

        ImageCreateInfo imgCreateInfo;
        imgCreateInfo.setImageType(ImageType::e2D).setFormat(depthFmt).setMipLevels(1)
                .setArrayLayers(1).setSamples(SampleCountFlagBits::e1).setTiling(ImageTiling::eOptimal)
                .setUsage(ImageUsageFlagBits::eDepthStencilAttachment)
                .setExtent(Extent3D(m_width,m_height,1));

        ImageViewCreateInfo imgViewCreateInfo;
        imgViewCreateInfo.setFormat(depthFmt)
                .setSubresourceRange(ImageSubresourceRange(ImageAspectFlagBits::eDepth,0,1,0,1))
                .setViewType(ImageViewType::e2D);

        bool  pass;

        m_depthBuffers[i].format = depthFmt;

        // Create the image with details as imageCreateInfo
        m_depthBuffers[i].image = device.createImage(imgCreateInfo);

        // discover what memory requirements are for this image.
//        vkGetImageMemoryRequirements(m_device, m_depthBuffers[i].image, &mem_reqs);

        MemoryRequirements memoryRequirements = device.getImageMemoryRequirements(m_depthBuffers[i].image);

        MemoryAllocateInfo memoryAllocateInfo(memoryRequirements.size, 0);
        pass = GetMemoryTypeFromProperties(memoryRequirements.memoryTypeBits, (MemoryPropertyFlagBits)0, &memoryAllocateInfo.memoryTypeIndex);

        m_depthBuffers[i].mem = device.allocateMemory(memoryAllocateInfo);

        device.bindImageMemory(m_depthBuffers[i].image, m_depthBuffers[i].mem, 0);

        // Create the view for this image
        imgViewCreateInfo.image = m_depthBuffers[i].image;
        m_depthBuffers[i].view = device.createImageView(imgViewCreateInfo);
    }

}

///////////////////////////////////////////////////////////////////////////////

void VkSample::InitCommandbuffers()
{
    CommandPoolCreateInfo commandPoolCreateInfo(CommandPoolCreateFlagBits::eResetCommandBuffer, m_queueFamilyIndex);
    commandPool = device.createCommandPool(commandPoolCreateInfo);

    CommandBufferAllocateInfo commandBufferAllocateInfo(commandPool, CommandBufferLevel::ePrimary,1);
    // Create render command buffers, one per swapchain image
    for (int i=0; i < m_swapchainImageCount; i++)
    {
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
    BufferCreateInfo bufCreateInfo;
    bufCreateInfo.setUsage(BufferUsageFlagBits::eVertexBuffer).setSize(sizeof(vb));
    m_vertices.buf = device.createBuffer(bufCreateInfo);

    // Obtain the memory requirements for this buffer.
    MemoryRequirements memoryRequirements = device.getBufferMemoryRequirements(m_vertices.buf);

    // And allocate memory according to those requirements.
    MemoryAllocateInfo memAllocInfo(memoryRequirements.size,0);
    pass = GetMemoryTypeFromProperties(memoryRequirements.memoryTypeBits, MemoryPropertyFlagBits::eHostVisible, &memAllocInfo.memoryTypeIndex);

    m_vertices.mem = device.allocateMemory(memAllocInfo);

    // Now we need to map the memory of this new allocation so the CPU can edit it.
    void *data = device.mapMemory(m_vertices.mem, 0, memAllocInfo.allocationSize);
//    VK_CHECK(!err);

    // Copy our triangle verticies and colors into the mapped memory area.
    memcpy(data, vb, sizeof(vb));

    // Unmap the memory back from the CPU.
    device.unmapMemory(m_vertices.mem);

    // Bind our buffer to the memory.
    device.bindBufferMemory(m_vertices.buf, m_vertices.mem, 0);

    // The vertices need to be defined so that the pipeline understands how the
    // data is laid out. This is done by providing a VkPipelineVertexInputStateCreateInfo
    // structure with the correct information.
    m_vertices.vi.setVertexBindingDescriptionCount(1).setVertexAttributeDescriptionCount(2)
            .setPVertexBindingDescriptions(m_vertices.vi_bindings)
            .setPVertexAttributeDescriptions(m_vertices.vi_attrs);

    // We bind the buffer as a whole, using the correct buffer ID.
    // This defines the stride for each element of the vertex array.
    m_vertices.vi_bindings[0].setBinding(VERTEX_BUFFER_BIND_ID).setStride(sizeof(vb[0])).setInputRate(VertexInputRate::eVertex);

    // Within each element, we define the attributes. At location 0,
    // the vertex positions, in float3 format, with offset 0 as they are
    // first in the array structure.
    m_vertices.vi_attrs[0].setBinding(VERTEX_BUFFER_BIND_ID)
            .setLocation(0)
            .setFormat(Format::eR32G32B32Sfloat)
            .setOffset(0);

    // The second location is the vertex colors, in RGBA float4 format.
    // These appear in each element in memory after the float3 vertex
    // positions, so the offset is set accordingly.
    m_vertices.vi_attrs[1].setBinding(VERTEX_BUFFER_BIND_ID)
            .setLocation(1)
            .setFormat(Format::eR32G32B32A32Sfloat)
            .setOffset(sizeof(float)*3);
}

///////////////////////////////////////////////////////////////////////////////

void VkSample::InitLayouts()
{

    DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo;
    descriptorLayout = device.createDescriptorSetLayout(descriptorSetLayoutCreateInfo);

    // Our pipeline layout simply points to the empty descriptor layout.
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
    AttachmentReference clrRef(0,ImageLayout::eColorAttachmentOptimal);
    AttachmentReference depthRef(1,ImageLayout::eDepthStencilAttachmentOptimal);

    // There can be multiple subpasses in a renderpass, but this example has only one.
    // We set the color and depth references at the grahics bind point in the pipeline.
    SubpassDescription subpassDesc;
    subpassDesc.setPipelineBindPoint(PipelineBindPoint::eGraphics)
            .setColorAttachmentCount(1).setPColorAttachments(&clrRef)
            .setPDepthStencilAttachment(&depthRef);

    // The renderpass itself is created with the number of subpasses, and the
    // list of attachments which those subpasses can reference.
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
    PipelineVertexInputStateCreateInfo vi = m_vertices.vi;

    // Our vertex buffer describes a triangle list.
    PipelineInputAssemblyStateCreateInfo ia;
    ia.setTopology(PrimitiveTopology::eTriangleList);

    // State for rasterization, such as polygon fill mode is defined.
    PipelineRasterizationStateCreateInfo rs;
    rs.setPolygonMode(PolygonMode::eFill)
            .setCullMode(CullModeFlagBits::eBack)
            .setFrontFace(FrontFace::eClockwise)
            .setDepthClampEnable(VK_FALSE)
            .setDepthBiasEnable(VK_FALSE)
            .setRasterizerDiscardEnable(VK_FALSE);

    // For this example we do not do blending, so it is disabled.
    PipelineColorBlendAttachmentState attachmentState[1];
    attachmentState[0].setColorWriteMask((ColorComponentFlagBits)0xf);
    attachmentState[0].setBlendEnable(VK_FALSE);

    PipelineColorBlendStateCreateInfo cb;
    cb.setAttachmentCount(1).setPAttachments(attachmentState);

    // We define a simple viewport and scissor. It does not change during rendering
    // in this sample.
    Viewport viewPort(0,0,(float)m_width,(float)m_height,0.f,1.f);
    Rect2D scissor(Offset2D(0,0),Extent2D(m_width, m_height));
    PipelineViewportStateCreateInfo vp;
    vp.setScissorCount(1).setPScissors(&scissor)
            .setViewportCount(1).setPViewports(&viewPort);

    PipelineDepthStencilStateCreateInfo ds;
    ds.setDepthTestEnable(VK_TRUE).setDepthWriteEnable(VK_TRUE).setDepthCompareOp(CompareOp::eLessOrEqual)
            .setDepthBoundsTestEnable(VK_FALSE).setBack(StencilOpState(StencilOp::eKeep,StencilOp::eKeep,StencilOp::eKeep,CompareOp::eAlways))
            .setStencilTestEnable(VK_FALSE).setFront(StencilOpState(StencilOp::eKeep,StencilOp::eKeep,StencilOp::eKeep,CompareOp::eAlways));

    // We do not use multisample
    PipelineMultisampleStateCreateInfo ms;
    ms.setRasterizationSamples(SampleCountFlagBits::e1);

    // We define two shader stages: our vertex and fragment shader.
    // they are embedded as SPIR-V into a header file for ease of deployment.
    PipelineShaderStageCreateInfo stages[2];
    stages[0].setStage(ShaderStageFlagBits::eVertex)
            .setModule(CreateShaderModule((const uint32_t*)&shader_tri_vert[0], shader_tri_vert_size))
            .setPName("main");
    stages[1].setStage(ShaderStageFlagBits::eFragment)
            .setModule(CreateShaderModule((const uint32_t*)&shader_tri_frag[0], shader_tri_frag_size))
            .setPName("main");

    // Pipelines are allocated from pipeline caches.
    PipelineCacheCreateInfo pipelineCacheInfo;
    PipelineCache pipelineCache = device.createPipelineCache(pipelineCacheInfo);

    // Out graphics pipeline records all state information, including our renderpass
    // and pipeline layout. We do not have any dynamic state in this example.
    GraphicsPipelineCreateInfo pipeInfo;
    pipeInfo.setLayout(pipelineLayout).setPVertexInputState(&vi).setPInputAssemblyState(&ia)
            .setPRasterizationState(&rs).setPColorBlendState(&cb).setPMultisampleState(&ms)
            .setPViewportState(&vp).setPDepthStencilState(&ds)
            .setRenderPass(renderPass).setStageCount(2).setPStages(stages);
    pipeline = device.createGraphicsPipeline(pipelineCache,pipeInfo);

    // We can destroy the cache now as we do not need it. The shader modules also
    // can be destroyed after the pipeline is created.
    device.destroyPipelineCache(pipelineCache);
    device.destroyShaderModule(stages[0].module);
    device.destroyShaderModule(stages[1].module);
}
///////////////////////////////////////////////////////////////////////////////

ShaderModule VkSample::CreateShaderModule(const uint32_t* code, uint32_t size)
{
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
    ImageView attachments[2] = {};
    FramebufferCreateInfo fboInfo;
    fboInfo.setRenderPass(renderPass)
            .setAttachmentCount(2).setPAttachments(attachments)
            .setWidth(m_width).setHeight(m_height).setLayers(1);

    framebuffers.resize(m_swapchainImageCount);
    // Reusing the framebufferCreateInfo to create m_swapchainImageCount framebuffers,
    // only the attachments to the relevent image views change each time.
    for (uint32_t i = 0; i < m_swapchainImageCount; i++) {
        attachments[0] = m_swapchainBuffers[i].view;
        attachments[1] = m_depthBuffers[i].view;
        framebuffers[i] = device.createFramebuffer(fboInfo);
    }
}

///////////////////////////////////////////////////////////////////////////////

void VkSample::InitSync()
{
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
    Result ret = device.acquireNextImageKHR(swapChain, UINT64_MAX, backBufferSemaphore, Fence(), &m_swapchainCurrentIdx);
}

///////////////////////////////////////////////////////////////////////////////

void VkSample::PresentBackBuffer()
{
    // Use WSI to present. The semaphore chain used to signal rendering
    // completion allows the operation to wait before the present is
    // completed.
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
    delete [] m_depthBuffers;
    device.destroyPipeline(pipeline);
//    vkDestroyRenderPass(m_device, m_renderPass, nullptr);
    device.destroyRenderPass(renderPass);
//    vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);
    device.destroyPipelineLayout(pipelineLayout);
//    vkDestroyDescriptorSetLayout(m_device, m_descriptorLayout, nullptr);
    device.destroyDescriptorSetLayout(descriptorLayout);
    device.destroyCommandPool(commandPool);
    device.destroySemaphore(backBufferSemaphore);
    device.destroySemaphore(renderCompleteSemaphore);
    device.destroyBuffer(m_vertices.buf);
    device.freeMemory(m_vertices.mem);
    device.destroySwapchainKHR(swapChain);
    device.destroy();
    instance.destroySurfaceKHR(surface);
    instance.destroy();

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
        cmdBuffer.reset(CommandBufferResetFlagBits(0));
        CommandBufferInheritanceInfo inheritanceInfo;
        CommandBufferBeginInfo cmdBeginInfo(CommandBufferUsageFlagBits::eSimultaneousUse);
        //cmdBeginInfo.setPInheritanceInfo(&inheritanceInfo);

        // By calling vkBeginCommandBuffer, cmdBuffer is put into the recording state.
        cmdBuffer.begin(cmdBeginInfo);


        // Before we can use the back buffer from the swapchain, we must change the
        // image layout from the PRESENT mode to the COLOR_ATTACHMENT mode.
        // PRESENT mode is optimal for sending to the screen for users to see, so the
        // image will be set back to that mode after we have completed rendering.
        ImageMemoryBarrier preRenderBarrier(AccessFlagBits::eMemoryRead,AccessFlagBits::eColorAttachmentWrite,
                                      ImageLayout::ePresentSrcKHR,ImageLayout::eColorAttachmentOptimal,
                                      VK_QUEUE_FAMILY_IGNORED,VK_QUEUE_FAMILY_IGNORED,
                                      m_swapchainBuffers[i].image,
                                        ImageSubresourceRange(ImageAspectFlagBits::eColor,1,1,0,0));

        // Thie PipelineBarrier function can operate on memoryBarriers,
        // bufferMemory and imageMemory buffers. We only provide a single
        // imageMemoryBarrier.
        cmdBuffer.pipelineBarrier(PipelineStageFlagBits::eColorAttachmentOutput,PipelineStageFlagBits::eColorAttachmentOutput,
                                  DependencyFlags(),0,nullptr,0,nullptr,1,&preRenderBarrier);
                             
        // When starting the render pass, we can set clear values.
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
        cmdBuffer.bindPipeline(PipelineBindPoint::eGraphics,pipeline);

        // Bind our vertex buffer, with a 0 offset.
        VkDeviceSize offsets[1] = {0};
        cmdBuffer.bindVertexBuffers(VERTEX_BUFFER_BIND_ID, 1, &m_vertices.buf, offsets);
        cmdBuffer.draw(3,1,0,0);
        cmdBuffer.endRenderPass();

        // As stated earlier, now transition the swapchain image to the PRESENT mode.
        ImageMemoryBarrier prePresentBarrier(AccessFlagBits::eColorAttachmentWrite,AccessFlagBits::eMemoryRead,
                                            ImageLayout::eColorAttachmentOptimal,ImageLayout::ePresentSrcKHR,
                                            VK_QUEUE_FAMILY_IGNORED,VK_QUEUE_FAMILY_IGNORED,
                                            m_swapchainBuffers[i].image,
                                            ImageSubresourceRange(ImageAspectFlagBits::eColor,1,1,0,0));
        cmdBuffer.pipelineBarrier(PipelineStageFlagBits::eColorAttachmentOutput,PipelineStageFlagBits::eBottomOfPipe,
                                  DependencyFlags(),0,nullptr,0,nullptr,1,&prePresentBarrier);
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
