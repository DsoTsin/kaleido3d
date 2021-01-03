#include "vk_common.h"
#include "vk_format.h"

namespace vulkan {

    // ---- GpuBufferView ----

	GpuBufferView::GpuBufferView(GpuBuffer* buffer)
		: buffer_(buffer)
	{
	}

	GpuBufferView::~GpuBufferView()
	{
	}

	const ngfx::Buffer* GpuBufferView::buffer() const
	{
		return buffer_;
	}

	ngfx::BufferUsage GpuBufferView::usage() const
	{
		return buffer_->desc().usages;
	}

    // ---- GpuBuffer ----

    GpuBuffer::GpuBuffer(const ngfx::BufferDesc& desc, VkBuffer buffer, GpuAllocator::MemoryItem inMemItem, GpuDevice* device)
		: buffer_(buffer)
		, desc_(desc)
		, mem_item_(inMemItem)
		, device_(device)
    {
    }

    GpuBuffer::~GpuBuffer()
    {
		if (buffer_)
		{
			device_->getAllocator().freeBuffer(buffer_, mem_item_);
			device_->destroyBuffer(buffer_);
			buffer_ = VK_NULL_HANDLE;
		}
    }

    ngfx::BufferView* GpuBuffer::newView(ngfx::Result * result)
    {
		if (result) *result = ngfx::Result::Ok;
		return new GpuBufferView(this);
    }

	void* GpuBuffer::map(ngfx::uint64 offset, ngfx::uint64 size)
	{
		return device_->mapMemory(mem_item_.mem, mem_item_.offset + offset, size, 0);
	}

	void GpuBuffer::unmap(void * addr)
	{
		device_->unmapMemory(mem_item_.mem);
	}

	void GpuBuffer::setLabel(const char * label)
	{
		if (label) label_ = label;
	}

	const char* GpuBuffer::label() const
	{
		return label_.empty() ? nullptr : label_.c_str();
	}

    // ---- GpuTextureView ----

	GpuTextureView::GpuTextureView(GpuTexture* texture, VkImageView view)
		: texture_(texture), view_(view)
	{
	}

	GpuTextureView::~GpuTextureView()
	{
	}

	const ngfx::Texture* GpuTextureView::texture() const
	{
		return texture_;
	}

	ngfx::TextureUsage GpuTextureView::usage() const
	{
		return texture_->desc().usages;
	}

    // ---- GpuTexture ----

    GpuTexture::GpuTexture(const ngfx::TextureDesc& desc, VkImage image, GpuAllocator::MemoryItem inMemItem, GpuDevice * device)
		: texture_(image)
		, desc_(desc)
		, mem_item_(inMemItem)
		, owns_image_(true)
		, device_(device)
    {
    }

	GpuTexture::GpuTexture(const ngfx::TextureDesc& desc, VkImage image, GpuDevice* device)
		: texture_(image)
		, desc_(desc)
		, mem_item_{}
		, owns_image_(false)
		, device_(device)
	{
	}

    GpuTexture::~GpuTexture()
    {
		if (texture_ && owns_image_)
		{
			device_->getAllocator().freeImage(texture_, mem_item_);
			device_->destroyImage(texture_);
			texture_ = VK_NULL_HANDLE;
		}
    }

	ngfx::PixelFormat GpuTexture::format() const
	{
		return desc_.format;
	}

	ngfx::TextureView* GpuTexture::newView(ngfx::Result * result)
    {
		VkImageViewCreateInfo viewInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
		viewInfo.image = texture_;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = pixelFormatToVk(desc_.format);
		viewInfo.subresourceRange.aspectMask = imageAspect(desc_.format);
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = desc_.mipLevels ? desc_.mipLevels : 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = desc_.layers ? desc_.layers : 1;

		VkImageView view = VK_NULL_HANDLE;
		VkResult ret = device_->createImageView(&viewInfo, &view);
		if (ret == VK_SUCCESS) {
			if (result) *result = ngfx::Result::Ok;
			return new GpuTextureView(this, view);
		}
		if (result) *result = ngfx::Result::Failed;
		return nullptr;
    }

	void* GpuTexture::map(ngfx::uint64 offset, ngfx::uint64 size)
	{
		if (mem_item_.mem == VK_NULL_HANDLE) return nullptr;
		return device_->mapMemory(mem_item_.mem, mem_item_.offset + offset, size, 0);
	}

	void GpuTexture::unmap(void* addr)
	{
		if (mem_item_.mem != VK_NULL_HANDLE)
			device_->unmapMemory(mem_item_.mem);
	}

	void GpuTexture::setLabel(const char * label)
	{
		if (label) label_ = label;
	}

	const char* GpuTexture::label() const
	{
		return label_.empty() ? nullptr : label_.c_str();
	}

    // ---- GpuSampler ----

    GpuSampler::GpuSampler(const ngfx::SamplerDesc& desc, VkSampler sampler, GpuDevice * device)
        : desc_(desc)
		, sampler_(sampler)
        , device_(device)
    {
    }

    GpuSampler::~GpuSampler()
    {
		if (sampler_ != VK_NULL_HANDLE) {
			device_->destroySampler(sampler_);
			sampler_ = VK_NULL_HANDLE;
		}
    }

    void GpuSampler::setLabel(const char * label)
    {
		if (label) label_ = label;
    }

    const char * GpuSampler::label() const
    {
		return label_.empty() ? nullptr : label_.c_str();
    }

    // ---- Pipeline Base ----

    GpuPipelineBase::GpuPipelineBase(GpuDevice * device)
        : device_(device)
    {
    }

    GpuPipelineBase::~GpuPipelineBase()
    {
		if (pipeline_ != VK_NULL_HANDLE) {
			device_->destroyPipeline(pipeline_);
			pipeline_ = VK_NULL_HANDLE;
		}
		if (pipeline_layout_ != VK_NULL_HANDLE) {
			device_->destroyPipelineLayout(pipeline_layout_);
			pipeline_layout_ = VK_NULL_HANDLE;
		}
		if (pipeline_cache_ != VK_NULL_HANDLE) {
			device_->destroyPipelineCache(pipeline_cache_);
			pipeline_cache_ = VK_NULL_HANDLE;
		}
		if (descriptor_set_layout_ != VK_NULL_HANDLE) {
			device_->destroyDescriptorSetLayout(descriptor_set_layout_);
			descriptor_set_layout_ = VK_NULL_HANDLE;
		}
    }

	void GpuPipelineBase::setLabel(const char* label)
	{
		if (label) label_ = label;
	}

	const char* GpuPipelineBase::label() const
	{
		return label_.empty() ? nullptr : label_.c_str();
	}

	ngfx::BindGroup* GpuPipelineBase::newBindGroup(ngfx::Result* result)
	{
		if (descriptor_set_layout_ == VK_NULL_HANDLE) {
			if (result) *result = ngfx::Result::Failed;
			return nullptr;
		}
		VkDescriptorPoolSize poolSizes[] = {
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 16 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 16 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 8 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 4 },
		};
		VkDescriptorPoolCreateInfo poolInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
		poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		poolInfo.maxSets = 1;
		poolInfo.poolSizeCount = 4;
		poolInfo.pPoolSizes = poolSizes;

		VkDescriptorPool pool = VK_NULL_HANDLE;
		VkResult ret = device_->createDescriptorPool(&poolInfo, &pool);
		if (ret != VK_SUCCESS) {
			if (result) *result = ngfx::Result::Failed;
			return nullptr;
		}

		VkDescriptorSetAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
		allocInfo.descriptorPool = pool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &descriptor_set_layout_;

		VkDescriptorSet set = VK_NULL_HANDLE;
		ret = device_->allocateDescriptorSets(&allocInfo, &set);
		if (ret != VK_SUCCESS) {
			device_->destroyDescriptorPool(pool);
			if (result) *result = ngfx::Result::Failed;
			return nullptr;
		}

		if (result) *result = ngfx::Result::Ok;
		return new GpuBindGroup(device_.get(), pool, set, descriptor_set_layout_);
	}

	ngfx::Device* GpuPipelineBase::device()
	{
		return device_.get();
	}

    // ---- BindGroup ----

	GpuBindGroup::GpuBindGroup(GpuDevice* device, VkDescriptorPool pool, VkDescriptorSet set, VkDescriptorSetLayout layout)
		: descriptor_pool_(pool)
		, descriptor_set_(set)
		, descriptor_set_layout_(layout)
		, device_(device)
	{
	}

	GpuBindGroup::~GpuBindGroup()
	{
		if (descriptor_pool_ != VK_NULL_HANDLE) {
			device_->destroyDescriptorPool(descriptor_pool_);
			descriptor_pool_ = VK_NULL_HANDLE;
		}
	}

	void GpuBindGroup::setSampler(ngfx::uint32 id, ngfx::ShaderStage stage, const ngfx::Sampler* sampler)
	{
		if (!sampler) return;
		auto* gpuSampler = static_cast<const GpuSampler*>(sampler);
		VkDescriptorImageInfo imageInfo = {};
		imageInfo.sampler = gpuSampler->vkSampler();

		VkWriteDescriptorSet write = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		write.dstSet = descriptor_set_;
		write.dstBinding = id;
		write.dstArrayElement = 0;
		write.descriptorCount = 1;
		write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
		write.pImageInfo = &imageInfo;

		device_->updateDescriptorSets(1, &write, 0, nullptr);
	}

	void GpuBindGroup::setTexture(ngfx::uint32 id, ngfx::ShaderStage stage, const ngfx::TextureView* texture)
	{
		if (!texture) return;
		auto* gpuView = static_cast<const GpuTextureView*>(texture);
		VkDescriptorImageInfo imageInfo = {};
		imageInfo.imageView = gpuView->vkImageView();
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkWriteDescriptorSet write = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		write.dstSet = descriptor_set_;
		write.dstBinding = id;
		write.dstArrayElement = 0;
		write.descriptorCount = 1;
		write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		write.pImageInfo = &imageInfo;

		device_->updateDescriptorSets(1, &write, 0, nullptr);
	}

	void GpuBindGroup::setBuffer(ngfx::uint32 id, ngfx::ShaderStage stage, const ngfx::BufferView* buffer)
	{
		if (!buffer) return;
		auto* gpuView = static_cast<const GpuBufferView*>(buffer);
		auto* gpuBuf = gpuView->gpuBuffer();

		VkDescriptorBufferInfo bufferInfo = {};
		bufferInfo.buffer = gpuBuf->vkBuffer();
		bufferInfo.offset = 0;
		bufferInfo.range = gpuBuf->desc().size;

		VkWriteDescriptorSet write = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		write.dstSet = descriptor_set_;
		write.dstBinding = id;
		write.dstArrayElement = 0;
		write.descriptorCount = 1;
		write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		write.pBufferInfo = &bufferInfo;

		device_->updateDescriptorSets(1, &write, 0, nullptr);
	}

	void GpuBindGroup::setRaytracingAS(ngfx::uint32 id, ngfx::ShaderStage stage, const ngfx::RaytracingAS* as)
	{
		// NV raytracing descriptor writes handled via pNext chain
	}

    // ---- Buffer/Image creation helpers ----

	static VkBufferCreateInfo GetBufferCreateInfo(ngfx::BufferDesc const& desc, ngfx::StorageMode mode)
	{
		VkBufferCreateInfo CreateInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		CreateInfo.size = desc.size;
		CreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		if ((uint8_t)desc.usages & (uint8_t)ngfx::BufferUsage::VertexBuffer)
			CreateInfo.usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		if ((uint8_t)desc.usages & (uint8_t)ngfx::BufferUsage::IndexBuffer)
			CreateInfo.usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		if ((uint8_t)desc.usages & (uint8_t)ngfx::BufferUsage::UniformBuffer)
			CreateInfo.usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		if ((uint8_t)desc.usages & (uint8_t)ngfx::BufferUsage::UnorderedAccess)
			CreateInfo.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
		if ((uint8_t)desc.usages & (uint8_t)ngfx::BufferUsage::AccelerationStructure)
			CreateInfo.usage |= VK_BUFFER_USAGE_RAY_TRACING_BIT_NV;
		CreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		return CreateInfo;
	}

	static VkImageCreateInfo GetImageCreateInfo(ngfx::TextureDesc const& desc, ngfx::StorageMode mode)
	{
		VkImageCreateInfo CreateInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
		CreateInfo.imageType = VK_IMAGE_TYPE_2D;
		if (desc.depth > 1) CreateInfo.imageType = VK_IMAGE_TYPE_3D;
		CreateInfo.format = pixelFormatToVk(desc.format);
		CreateInfo.extent.width = desc.width > 0 ? desc.width : 1;
		CreateInfo.extent.height = desc.height > 0 ? desc.height : 1;
		CreateInfo.extent.depth = desc.depth > 0 ? desc.depth : 1;
		CreateInfo.mipLevels = desc.mipLevels > 0 ? desc.mipLevels : 1;
		CreateInfo.arrayLayers = desc.layers > 0 ? desc.layers : 1;
		CreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		CreateInfo.tiling = (mode == ngfx::StorageMode::Shared) ? VK_IMAGE_TILING_LINEAR : VK_IMAGE_TILING_OPTIMAL;
		if (isDepthStencilFormat(desc.format)) {
			CreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		} else {
			CreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
			if ((uint8_t)desc.usages & (uint8_t)ngfx::TextureUsage::RenderTarget)
				CreateInfo.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		}
		CreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		CreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		return CreateInfo;
	}

    ngfx::Texture* GpuDevice::newTexture(const ngfx::TextureDesc * desc, ngfx::StorageMode mode, ngfx::Result * result)
    {
		VkImageCreateInfo info = GetImageCreateInfo(*desc, mode);
		VkImage image = VK_NULL_HANDLE;
		VkResult ret = this->createImage(&info, &image);
		if (ret == VK_SUCCESS)
		{
			GpuAllocator::MemoryItem item;
			mem_alloc_.allocateForImage(image, mode, item);
			this->__BindImageMemory(device_, image, item.mem, item.offset);
			if (result) *result = ngfx::Result::Ok;
			return new GpuTexture(*desc, image, item, this);
		}
		if (result) *result = ngfx::Result::Failed;
		return nullptr;
    }

    ngfx::Buffer* GpuDevice::newBuffer(const ngfx::BufferDesc* desc, ngfx::StorageMode mode, ngfx::Result* result)
    {
		check(desc);
		check(desc->size > 0);
		VkBuffer buffer = VK_NULL_HANDLE;
		auto info = GetBufferCreateInfo(*desc, mode);
		VkResult ret = this->createBuffer(&info, &buffer);
		if (ret == VK_SUCCESS)
		{
			GpuAllocator::MemoryItem item;
			mem_alloc_.allocateForBuffer(buffer, mode, item);
			this->__BindBufferMemory(device_, buffer, item.mem, item.offset);
			if (result) *result = ngfx::Result::Ok;
			return new GpuBuffer(*desc, buffer, item, this);
		}
		if (result) *result = ngfx::Result::Failed;
		return nullptr;
    }
    
    ngfx::Sampler* GpuDevice::newSampler(const ngfx::SamplerDesc * desc, ngfx::Result * result) {
		VkSamplerCreateInfo samplerInfo = { VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
		samplerInfo.magFilter = filterModeToVk(desc->magFilter);
		samplerInfo.minFilter = filterModeToVk(desc->minFilter);
		samplerInfo.mipmapMode = mipFilterToVk(desc->mipMapFilter);
		samplerInfo.addressModeU = addressModeToVk(desc->u);
		samplerInfo.addressModeV = addressModeToVk(desc->v);
		samplerInfo.addressModeW = addressModeToVk(desc->w);
		samplerInfo.mipLodBias = desc->minLodBias;
		samplerInfo.anisotropyEnable = desc->maxAnistropy > 1 ? VK_TRUE : VK_FALSE;
		samplerInfo.maxAnisotropy = (float)desc->maxAnistropy;
		samplerInfo.compareEnable = (desc->comparisonFunction != ngfx::ComparisonFunction::Never) ? VK_TRUE : VK_FALSE;
		samplerInfo.compareOp = compareOpToVk(desc->comparisonFunction);
		samplerInfo.minLod = desc->minLod;
		samplerInfo.maxLod = desc->maxLod;
		samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;

		VkSampler sampler = VK_NULL_HANDLE;
		VkResult ret = this->createSampler(&samplerInfo, &sampler);
		if (ret == VK_SUCCESS) {
			if (result) *result = ngfx::Result::Ok;
			return new GpuSampler(*desc, sampler, this);
		}
		if (result) *result = ngfx::Result::Failed;
		return nullptr;
    }
}
