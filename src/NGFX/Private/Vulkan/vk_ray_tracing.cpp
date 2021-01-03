#include "vk_common.h"

namespace vulkan {

    VkMemoryRequirements GpuDevice::getAccelerationStructureMemorySize(VkAccelerationStructureNV accel)
    {
        VkAccelerationStructureMemoryRequirementsInfoNV memoryRequirementsInfo;
        memoryRequirementsInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_NV;
        memoryRequirementsInfo.pNext = nullptr;
        memoryRequirementsInfo.accelerationStructure = accel;
        memoryRequirementsInfo.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_NV;
        VkMemoryRequirements2 memoryRequirements = {};
        if (this->__GetAccelerationStructureMemoryRequirementsNV) {
            this->__GetAccelerationStructureMemoryRequirementsNV(device_, &memoryRequirementsInfo, &memoryRequirements);
        }
        return memoryRequirements.memoryRequirements;
    }

    GpuRaytracingAccelerationStructure::GpuRaytracingAccelerationStructure(
		const ngfx::RaytracingASDesc& desc,
		VkAccelerationStructureNV inAs,
		GpuAllocator::MemoryItem const& memItem, GpuDevice* device)
        : acceleration_structure_(inAs)
		, mem_item_(memItem)
		, desc_(desc)
        , device_(device)
    {
        create_info_ = {
            VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_NV
        };
    }

    GpuRaytracingAccelerationStructure::~GpuRaytracingAccelerationStructure()
    {
		if (acceleration_structure_ != VK_NULL_HANDLE) {
			device_->getAllocator().freeAccelerationStructure(acceleration_structure_, mem_item_);
			device_->destroyAccelerationStructure(acceleration_structure_, NGFXVK_ALLOCATOR);
			acceleration_structure_ = VK_NULL_HANDLE;
		}
    }

    void GpuRaytracingAccelerationStructure::setLabel(const char * label)
    {
		if (label) label_ = label;
    }

    const char * GpuRaytracingAccelerationStructure::label() const
    {
		return label_.empty() ? nullptr : label_.c_str();
    }

    ngfx::RaytracingAS* GpuDevice::newRaytracingAS(const ngfx::RaytracingASDesc* rtDesc, ngfx::Result * result)
    {
		VkAccelerationStructureCreateInfoNV as_info = { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_NV };
		VkAccelerationStructureNV as = VK_NULL_HANDLE;
		createAccelerationStructure(&as_info, NGFXVK_ALLOCATOR, &as);
		auto memoryRequirements = getAccelerationStructureMemorySize(as);
		GpuAllocator::MemoryItem memItem;
		mem_alloc_.allocateForAccelerationStructure(as, ngfx::StorageMode::Private, memItem);
		if (result) *result = ngfx::Result::Ok;
        return new GpuRaytracingAccelerationStructure(*rtDesc, as, memItem, this);
    }

    GpuRaytracingPipeline::GpuRaytracingPipeline(const ngfx::RaytracePipelineDesc * desc, GpuDevice * device)
        : GpuPipelineBase(device)
    {
        std::vector<VkRayTracingShaderGroupCreateInfoNV> shaderGroups({
            { VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV, nullptr, VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV, 0, VK_SHADER_UNUSED_NV, VK_SHADER_UNUSED_NV, VK_SHADER_UNUSED_NV },
            { VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV, nullptr, VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV, 1, VK_SHADER_UNUSED_NV, VK_SHADER_UNUSED_NV, VK_SHADER_UNUSED_NV },
            { VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV, nullptr, VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_NV, VK_SHADER_UNUSED_NV, 2, VK_SHADER_UNUSED_NV, VK_SHADER_UNUSED_NV },
            { VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV, nullptr, VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_NV, VK_SHADER_UNUSED_NV, 3, VK_SHADER_UNUSED_NV, VK_SHADER_UNUSED_NV },
            });

        create_info_ = {
            VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_NV
        };
        create_info_.groupCount = (uint32_t)shaderGroups.size();
        create_info_.pGroups = shaderGroups.data();
        create_info_.maxRecursionDepth = desc->maxTraceRecurseDepth;
        create_info_.basePipelineHandle = VK_NULL_HANDLE;
        create_info_.basePipelineIndex = 0;
    }

    GpuRaytracingPipeline::~GpuRaytracingPipeline()
    {
    }

    ngfx::RaytracePipeline* GpuDevice::newRaytracePipeline(const ngfx::RaytracePipelineDesc * desc, ngfx::Result * result)
    {
		if (!support_raytracing_) {
			if (result) *result = ngfx::Result::Failed;
			return nullptr;
		}
		auto* pipeline = new GpuRaytracingPipeline(desc, this);
		if (result) *result = ngfx::Result::Ok;
		return nullptr; // TODO: cast properly when GpuRaytracingPipeline inherits RaytracePipeline
    }
}
