#include "vk_common.h"
#include <unordered_map>

#define VULKAN_STANDARD_LAYER "VK_LAYER_LUNARG_standard_validation"

namespace vulkan {
    LayerEnumerator gLayerEnumerator;

	GpuDevice::GpuDevice(VkDevice device)
        : handle_(device)
	{
        volkLoadDeviceTable(this, device);
	}
	GpuDevice::~GpuDevice()
	{
	}
    ngfx::DeviceType GpuDevice::getType() const
    {
        return ngfx::DeviceType();
    }
	ngfx::CommandQueue* GpuDevice::newQueue()
	{
		return nullptr;
	}
	ngfx::Shader* GpuDevice::newShader()
	{
		return nullptr;
	}
	ngfx::Renderpass* GpuDevice::newRenderpass(const ngfx::RenderpassDesc * desc, ngfx::Result * result)
	{
		return nullptr;
	}
	ngfx::ComputePipeline* GpuDevice::newComputePipeline(const ngfx::ComputePipelineDesc * desc, ngfx::Result * result)
	{
		return nullptr;
	}
	ngfx::RaytracePipeline* GpuDevice::newRaytracePipeline(const ngfx::RaytracePipelineDesc * desc, ngfx::Result * result)
	{
		return nullptr;
	}
	ngfx::Texture* GpuDevice::newTexture(const ngfx::TextureDesc * desc, ngfx::StorageMode mode, ngfx::Result * result)
	{
		return nullptr;
	}
	ngfx::Buffer* GpuDevice::newBuffer(const ngfx::BufferDesc * desc, ngfx::StorageMode mode, ngfx::Result * result)
	{
		return nullptr;
	}
	ngfx::RaytracingAS* GpuDevice::newRaytracingAS(const ngfx::RaytracingASDesc * rtDesc, ngfx::Result * result)
	{
		return nullptr;
	}
	ngfx::Sampler* GpuDevice::newSampler(const ngfx::SamplerDesc * desc, ngfx::Result * result)
	{
		return nullptr;
	}
	ngfx::Fence* GpuDevice::newFence()
	{
		return nullptr;
	}
	ngfx::Result GpuDevice::wait()
	{
		return ngfx::Result();
	}

    GpuQueue::GpuQueue()
    {
    }
    GpuQueue::~GpuQueue()
    {
    }
    ngfx::CommandBuffer* GpuQueue::newCommandBuffer()
    {
        return nullptr;
    }

    GpuFactory::GpuFactory(VkInstance instance, bool debug_enable, ngfx_LogCallback log_call)
        : instance_(instance)
        , debug_enable_(debug_enable)
        , log_call_(log_call)
    {
        volkLoadInstance(instance_);
        uint32_t num_devs = 0;
        vkEnumeratePhysicalDevices(instance_, &num_devs, nullptr);
        if (num_devs > 0) {
            ngfx::Vec<VkPhysicalDevice> devs;
            devs.resize(num_devs);
            vkEnumeratePhysicalDevices(instance_, &num_devs, &devs[0]);
            for (auto& dev : devs) {
                VkPhysicalDeviceFeatures features = {};
                VkPhysicalDeviceProperties properties = {};
                vkGetPhysicalDeviceFeatures(dev, &features);
                vkGetPhysicalDeviceProperties(dev, &properties);
                properties.deviceType;
                properties.deviceName;
                VkDeviceCreateInfo create_info = { 
                    VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO, nullptr, 0
                };
                VkDevice device = VK_NULL_HANDLE;
                vkCreateDevice(dev, &create_info, &gAllocationCallbacks, &device);
            }
        }
    }

    GpuFactory::~GpuFactory()
    {
    }

    ngfx::Swapchain * GpuFactory::newSwapchain(void * handle, void * reserved)
    {
        return nullptr;
    }
    int GpuFactory::numDevices()
    {
        return 0;
    }
    ngfx::Device * GpuFactory::getDevice(ngfx::uint32 id)
    {
        return nullptr;
    }



    void LayerEnumerator::init()
    {
        ngfx::Vec< VkLayerProperties >      layer_props;
        uint32_t num_layers = 0;
        vkEnumerateInstanceLayerProperties(&num_layers, nullptr);
        if (num_layers > 0) {
            layers_.resize(num_layers);
            layer_props.resize(num_layers);
            vkEnumerateInstanceLayerProperties(&num_layers, &layer_props[0]);
            for (uint32_t id = 0; id < num_layers; id++) {
                auto& layer_prop = layer_props[id];
                memcpy(&layers_[id].props_, &layer_prop, sizeof(layer_prop));
                uint32_t num_props = 0;
                vkEnumerateInstanceExtensionProperties(layer_prop.layerName, &num_props, nullptr);
                if (num_props > 0) {
                    layers_[id].ext_props_.resize(num_props);
                    vkEnumerateInstanceExtensionProperties(layer_prop.layerName, &num_props, &layers_[id].ext_props_[0]);
                }
            }
        }
    }
}

ngfx::Factory* CreateFactory(bool debug_layer_enable, ngfx_LogCallback log_call)
{
    volkInitialize();

    vulkan::gLayerEnumerator.init();

    VkInstance instance = VK_NULL_HANDLE;

    VkApplicationInfo appInfo = { VK_STRUCTURE_TYPE_APPLICATION_INFO, nullptr, 
        "ngfx_vulkan", 1,
        "ngfx", 1,
        volkGetInstanceVersion()
    };

    VkInstanceCreateInfo instanceInfo = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO, nullptr,
        0, &appInfo
    };

    //if (debug_layer_enable) // Debug extension ??
    //{
    //    instanceCreateInfo.enabledLayerCount = RequiredLayers.size();
    //    instanceCreateInfo.ppEnabledLayerNames = RequiredLayers.data();
    //}

    //instanceCreateInfo.enabledExtensionCount = RequiredInstanceExtensions.size();
    //instanceCreateInfo.ppEnabledExtensionNames = RequiredInstanceExtensions.data();

    VkResult result = vkCreateInstance(&instanceInfo, &vulkan::gAllocationCallbacks, &instance);
    return new vulkan::GpuFactory(instance, debug_layer_enable, log_call);
}