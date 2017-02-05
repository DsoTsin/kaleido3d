#include "VkCommon.h"
#include "VkRHI.h"
#include "VkUtils.h"
#include "VkEnums.h"
#include <algorithm>

using namespace rhi;

K3D_VK_BEGIN

Resource::Ptr Resource::Map(uint64 offset, uint64 size)
{
	Resource::Ptr ptr;
	K3D_VK_VERIFY(GetGpuRef()->vkMapMemory(GetRawDevice(), m_DeviceMem, m_AllocationOffset+offset, size, 0, &ptr));
	return ptr;
}

Resource::~Resource()
{
	if (!m_DeviceMem)
		return;
	VKLOG(Info, "Resource freeing gpu memory. -- 0x%0x, tid:%d", m_DeviceMem, Os::Thread::GetId());
	GetGpuRef()->vkFreeMemory(GetRawDevice(), m_DeviceMem, nullptr);
	m_DeviceMem = VK_NULL_HANDLE;
}

Buffer::Buffer(Device::Ptr pDevice, rhi::ResourceDesc const &desc)
: Resource(pDevice, desc)
{
	m_Usage = g_ResourceViewFlag[desc.ViewType];
	
	if (desc.CreationFlag & rhi::EGRCF_TransferSrc)
	{
		m_Usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	}
	
	if (desc.CreationFlag & rhi::EGRCF_TransferDst) 
	{
		m_Usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	}

	if (desc.Flag & rhi::EGRAF_HostVisible) 
	{
		m_MemoryBits |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
	}
	if (desc.Flag & rhi::EGRAF_DeviceVisible)
	{
		m_MemoryBits |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	}
	if (desc.Flag & rhi::EGRAF_HostCoherent)
	{
		m_MemoryBits |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	}
	Create(desc.Size);
}

Buffer::~Buffer()
{
	if (VK_NULL_HANDLE != m_BufferView)
	{
		GetGpuRef()->vkDestroyBufferView(GetRawDevice(), m_BufferView, nullptr);
		m_BufferView = VK_NULL_HANDLE;
	}
	if (VK_NULL_HANDLE != m_Buffer)
	{
		VKLOG(Info, "Buffer Destroying.. -- %0x.", m_Buffer);
		GetGpuRef()->vkDestroyBuffer(GetRawDevice(), m_Buffer, nullptr);
		m_Buffer = VK_NULL_HANDLE;
	}
}

void Buffer::Create(size_t size)
{
	VkBufferCreateInfo createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.size = size;
	createInfo.usage = m_Usage;
	createInfo.flags = 0;
	createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	createInfo.queueFamilyIndexCount = 0;
	createInfo.pQueueFamilyIndices = nullptr;
	K3D_VK_VERIFY(GetGpuRef()->vkCreateBuffer(GetRawDevice(), &createInfo, nullptr, &m_Buffer));
	//ResourceManager::Allocation alloc = GetDevice()->GetMemoryManager()->AllocateBuffer(m_Buffer, false, m_MemoryBits);
	//K3D_ASSERT(VK_NULL_HANDLE != alloc.Memory);

	VkMemoryAllocateInfo memAlloc = {};
	memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	VkMemoryRequirements memReqs;
	GetGpuRef()->vkGetBufferMemoryRequirements(GetRawDevice(), m_Buffer, &memReqs);
	memAlloc.allocationSize = memReqs.size;
	GetDevice()->FindMemoryType(memReqs.memoryTypeBits, m_MemoryBits, &memAlloc.memoryTypeIndex);
	K3D_VK_VERIFY(GetGpuRef()->vkAllocateMemory(GetRawDevice(), &memAlloc, nullptr, &m_DeviceMem));

	m_AllocationOffset = 0;
	m_AllocationSize = memAlloc.allocationSize;
	m_Size = memAlloc.allocationSize;

	VKLOG(Info, "Buffer reqSize:(%d) allocated:(%d) offset:(%d) address:(0x%0x).",
		  m_Size, m_AllocationSize, m_AllocationOffset, m_DeviceMem);

	m_BufferInfo.buffer = m_Buffer;
	m_BufferInfo.offset = 0;
	m_BufferInfo.range = m_Size;
	
	K3D_VK_VERIFY(GetGpuRef()->vkBindBufferMemory(GetRawDevice(), m_Buffer, m_DeviceMem, m_AllocationOffset));
}

/*Texture::Texture(Device::Ptr pDevice, rhi::TextureDesc const &desc)
: Resource(pDevice)
{
	Create(desc);
}*/

Texture::Texture(Device::Ptr pDevice, rhi::ResourceDesc const & desc)
	: Resource(pDevice, desc)
{
	if (desc.CreationFlag & rhi::EGRCF_TransferDst)
	{
		m_ImageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	}
	if (desc.Flag & rhi::EGRAF_HostVisible)
	{
		m_MemoryBits |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
	}
	if (desc.Flag & rhi::EGRAF_DeviceVisible)
	{
		m_MemoryBits |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	}
	if (desc.Flag & rhi::EGRAF_HostCoherent)
	{
		m_MemoryBits |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	}
	switch (desc.ViewType)
	{
	case rhi::EGVT_SRV:
		m_SubResRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, desc.TextureDesc.MipLevels, 0, desc.TextureDesc.Layers };
		CreateSampledTexture(desc.TextureDesc);
		break;
	case rhi::EGVT_RTV:
		m_SubResRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, desc.TextureDesc.MipLevels, 0, desc.TextureDesc.Layers };
		CreateRenderTexture(desc.TextureDesc);
		break;
	case rhi::EGVT_DSV:
		m_SubResRange = { VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 0, desc.TextureDesc.MipLevels, 0, desc.TextureDesc.Layers };
		CreateRenderTexture(desc.TextureDesc);
		break;
	}
}

Texture::Texture(VkImage image, VkImageView imageView, VkImageViewCreateInfo info, Device::Ptr pDevice, bool selfOwnShip)
	: Resource(pDevice)
	, m_ImageView(imageView)
	, m_Image(image)
	, m_ImageViewInfo(info)
	, m_SelfOwn(selfOwnShip)
{
	m_SubResRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
}

void Texture::BindSampler(rhi::SamplerRef sampler)
{
	m_ImageSampler = k3d::StaticPointerCast<Sampler>(sampler);
}

SamplerCRef Texture::GetSampler() const
{
	return m_ImageSampler;
}

void Texture::CreateResourceView()
{
	m_ImageViewInfo = ImageViewInfo::From(m_ImageInfo, m_Image);
	K3D_VK_VERIFY(vkCreateImageView(GetRawDevice(), &m_ImageViewInfo, nullptr, &m_ImageView));
}

void Texture::CreateRenderTexture(TextureDesc const & desc)
{
	m_ImageInfo = ImageInfo::FromRHI(desc);
	K3D_VK_VERIFY(GetGpuRef()->vkCreateImage(GetRawDevice(), &m_ImageInfo, nullptr, &m_Image));

	//ResourceManager::Allocation alloc = GetDevice()->GetMemoryManager()->AllocateImage(m_Image, false, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
	//K3D_ASSERT(VK_NULL_HANDLE != alloc.Memory);

	VkMemoryAllocateInfo memAlloc = {};
	memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	VkMemoryRequirements memReqs;
	GetGpuRef()->vkGetImageMemoryRequirements(GetRawDevice(), m_Image, &memReqs);
	memAlloc.allocationSize = memReqs.size;
	GetDevice()->FindMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &memAlloc.memoryTypeIndex);
	K3D_VK_VERIFY(GetGpuRef()->vkAllocateMemory(GetRawDevice(), &memAlloc, nullptr, &m_DeviceMem));

	m_AllocationOffset = 0;
	m_AllocationSize = memAlloc.allocationSize;
	m_Size = memAlloc.allocationSize;

	K3D_VK_VERIFY(GetGpuRef()->vkBindImageMemory(GetRawDevice(), m_Image, m_DeviceMem, m_AllocationOffset));

	m_ImageViewInfo = ImageViewInfo::From(m_ImageInfo, m_Image);
	K3D_VK_VERIFY(GetGpuRef()->vkCreateImageView(GetRawDevice(), &m_ImageViewInfo, nullptr, &m_ImageView));

	VkImageSubresource subres;
	subres.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subres.mipLevel = 0; // temp use
	subres.arrayLayer = 0; // temp use
	// query texture memory layout info here
	// image must have been created with tiling equal to VK_IMAGE_TILING_LINEAR
	// The aspectMask member of pSubresource must only have a single bit set
	GetGpuRef()->vkGetImageSubresourceLayout(GetRawDevice(), m_Image, &subres, &m_SubResourceLayout);
}

void Texture::CreateDepthStencilTexture(TextureDesc const & desc)
{
}

void Texture::CreateSampledTexture(TextureDesc const & desc)
{
	m_ImageInfo = ImageInfo::FromRHI(desc);
	m_ImageInfo.usage = m_ImageUsage | VK_IMAGE_USAGE_SAMPLED_BIT;
	if (m_ImageUsage & VK_IMAGE_USAGE_TRANSFER_DST_BIT) // texture upload use staging
	{
		m_ImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		m_ImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	}
	else // directly upload
	{
		m_ImageInfo.tiling = VK_IMAGE_TILING_LINEAR;
		m_ImageInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
	}
	
	m_SubResRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, desc.MipLevels, 0, desc.Layers };

	K3D_VK_VERIFY(GetGpuRef()->vkCreateImage(GetRawDevice(), &m_ImageInfo, nullptr, &m_Image));

	//ResourceManager::Allocation alloc = GetDevice()->GetMemoryManager()->AllocateImage(m_Image, false, m_MemoryBits);
	//K3D_ASSERT(VK_NULL_HANDLE != alloc.Memory);

	VkMemoryAllocateInfo memAlloc = {};
	memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	VkMemoryRequirements memReqs;
	GetGpuRef()->vkGetImageMemoryRequirements(GetRawDevice(), m_Image, &memReqs);
	memAlloc.allocationSize = memReqs.size;
	GetDevice()->FindMemoryType(memReqs.memoryTypeBits, m_MemoryBits, &memAlloc.memoryTypeIndex);
	K3D_VK_VERIFY(GetGpuRef()->vkAllocateMemory(GetRawDevice(), &memAlloc, nullptr, &m_DeviceMem));

	m_AllocationOffset = 0;
	m_AllocationSize = memAlloc.allocationSize;
	m_Size = memAlloc.allocationSize;

	K3D_VK_VERIFY(GetGpuRef()->vkBindImageMemory(GetRawDevice(), m_Image, m_DeviceMem, m_AllocationOffset));
}

Texture::~Texture()
{
	if (GetRawDevice() == VK_NULL_HANDLE)
		return;
	if (VK_NULL_HANDLE != m_ImageView)
	{
		GetGpuRef()->vkDestroyImageView(GetRawDevice(), m_ImageView, nullptr);
		m_ImageView = VK_NULL_HANDLE;
	}
	if (VK_NULL_HANDLE != m_Image && m_SelfOwn)
	{
		GetGpuRef()->vkDestroyImage(GetRawDevice(), m_Image, nullptr);
		VKLOG(Info, "Texture Destroyed.. -- %0x.", m_Image);
		m_Image = VK_NULL_HANDLE;
	}
}

Texture::TextureRef Texture::CreateFromSwapChain(VkImage image, VkImageView view, VkImageViewCreateInfo info, Device::Ptr pDevice)
{
	return k3d::MakeShared<Texture>(image, view, info, pDevice, false);
}

ShaderResourceView::ShaderResourceView(rhi::ResourceViewDesc const &desc, rhi::GpuResourceRef pGpuResource)
	: m_Desc(desc), m_Resource(pGpuResource), m_TextureViewInfo{}, m_TextureView(VK_NULL_HANDLE)
{
	auto resourceDesc = m_Resource->GetResourceDesc();
	m_TextureViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	m_TextureViewInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
	switch(resourceDesc.Type)
	{
	case rhi::EGT_Texture1D:
		m_TextureViewInfo.viewType = VK_IMAGE_VIEW_TYPE_1D;
		break;
	case rhi::EGT_Texture2D:
		m_TextureViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		break;
	case rhi::EGT_Texture3D:
		m_TextureViewInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
		break;
	case rhi::EGT_Texture2DArray:
		m_TextureViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
		break;
	}
	m_TextureViewInfo.format = g_FormatTable[resourceDesc.TextureDesc.Format];
	m_TextureViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	m_TextureViewInfo.subresourceRange.baseMipLevel = 0;
	m_TextureViewInfo.subresourceRange.baseArrayLayer = 0;
	m_TextureViewInfo.subresourceRange.layerCount = 1;
	m_TextureViewInfo.subresourceRange.levelCount = resourceDesc.TextureDesc.MipLevels;
	m_TextureViewInfo.image = (VkImage)m_Resource->GetResourceLocation();
	K3D_VK_VERIFY(DynamicPointerCast<Resource>(m_Resource)->GetGpuRef()->
		vkCreateImageView(DynamicPointerCast<Resource>(m_Resource)->GetRawDevice(), &m_TextureViewInfo, nullptr, &m_TextureView));
}

ShaderResourceView::~ShaderResourceView()
{
	// destroy view
	if (m_TextureView)
	{
		vkDestroyImageView(DynamicPointerCast<Resource>(m_Resource)->GetRawDevice(), m_TextureView, nullptr);
		m_TextureView = VK_NULL_HANDLE;
	}
}

template<typename VkObject>
ResourceManager::Allocation
ResourceManager::Pool<VkObject>::Allocate(const typename ResourceManager::ResDesc<VkObject>& objDesc)
{
	ResourceManager::Allocation result = {};
	const VkMemoryRequirements& memReqs = objDesc.MemoryRequirements;
	if (HasAvailable(memReqs))
	{
		const VkDeviceSize initialOffset = m_Offset;
		const VkDeviceSize alignedOffset = CalcAlignedOffset(initialOffset, memReqs.alignment);
		const VkDeviceSize allocatedSize = memReqs.size;
		result.Memory = m_Memory;
		result.Offset = alignedOffset;
		result.Size = allocatedSize;
		m_Allocations.push_back(result);
		m_Offset += allocatedSize;
	}
	return result;
}

template <typename VkObjectT>
ResourceManager::Pool<VkObjectT>::Pool(uint32 memTypeIndex, VkDeviceMemory mem, VkDeviceSize sz) : m_MemoryTypeIndex(memTypeIndex), m_Memory(mem), m_Size(sz)
{
}


template<typename VkObject>
std::unique_ptr< ResourceManager::Pool<VkObject> >
ResourceManager::Pool<VkObject>::Create(VkDevice device, const VkDeviceSize poolSize, const typename ResourceManager::ResDesc<VkObject>& objDesc)
{
	std::unique_ptr< ResourceManager::Pool<VkObject> > result;
	const uint32_t memoryTypeIndex = objDesc.MemoryTypeIndex;
	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.pNext = nullptr;
	allocInfo.allocationSize = poolSize;
	allocInfo.memoryTypeIndex = memoryTypeIndex;
	VkDeviceMemory memory = VK_NULL_HANDLE;
	VkResult res = vkAllocateMemory(device, &allocInfo, nullptr, &memory);
	if (VK_SUCCESS == res) {
		result = std::unique_ptr< ResourceManager::Pool<VkObject> >(new ResourceManager::Pool<VkObject>(memoryTypeIndex, memory, poolSize));
	}
	return result;
}

template <typename VkObjectT>
bool ResourceManager::Pool<VkObjectT>::HasAvailable(VkMemoryRequirements memReqs) const
{
	VkDeviceSize alignedOffst = CalcAlignedOffset(m_Offset, memReqs.alignment);
#ifdef min
#undef min
#endif
	VkDeviceSize remaining = m_Size - std::min(alignedOffst, m_Size);
	return memReqs.size <= remaining;
}

template <typename VkObjectT>
ResourceManager::PoolManager<VkObjectT>::PoolManager(Device::Ptr pDevice, VkDeviceSize poolSize)
	: DeviceChild(pDevice), m_PoolSize(poolSize)
{
}

template<typename VkObjectT>
ResourceManager::PoolManager<VkObjectT>::~PoolManager()
{
	Destroy();
}

template<typename VkObjectT>
void ResourceManager::PoolManager<VkObjectT>::Destroy()
{
	if (!GetRawDevice())
		return;
	::Os::Mutex::AutoLock lock(&m_Mutex);
	for (auto& pool : m_Pools) {
		vkFreeMemory(GetRawDevice(), pool->m_Memory, nullptr);
	}
	m_Pools.clear();
}

template<typename VkObjectT>
ResourceManager::Allocation
ResourceManager::PoolManager<VkObjectT>::Allocate(const typename ResourceManager::ResDesc<VkObjectT>& objDesc)
{
	::Os::Mutex::AutoLock lock(&m_Mutex);
	ResourceManager::Allocation result;
	if (objDesc.MemoryRequirements.size > m_PoolSize)
	{
		auto pool = ResourceManager::Pool<VkObjectT>::Create(GetRawDevice(), objDesc.MemoryRequirements.size, objDesc);
		if (pool) 
		{
			result = pool->Allocate(objDesc);
			m_Pools.push_back(std::move(pool));
		}
	}
	else {
		// Look to see if there's a pool that fits the requirements...
		auto it = std::find_if(
			std::begin(m_Pools),
			std::end(m_Pools),
			[objDesc](const ResourceManager::PoolManager<VkObjectT>::PoolRef& elem) -> bool 
		{
			bool isMemoryType = (elem->GetMemoryTypeIndex() == objDesc.MemoryTypeIndex);
			bool hasSpace = elem->HasAvailable(objDesc.MemoryRequirements);
			return isMemoryType && hasSpace;
		}
		);

		// ...if there is allocate from the available pool
		if (std::end(m_Pools) != it) 
		{
			auto& pool = *it;
			result = pool->Allocate(objDesc);
		}
		// ...otherwise create a new pool and allocate from it
		else 
		{
			auto pool = ResourceManager::Pool<VkObjectT>::Create(GetRawDevice(), m_PoolSize, objDesc);
			if (pool) 
			{
				result = pool->Allocate(objDesc);
				m_Pools.push_back(std::move(pool));
			}
		}
	}
	return result;
}


ResourceManager::ResourceManager(Device::Ptr pDevice, size_t bufferBlockSize, size_t imageBlockSize)
	: DeviceChild(pDevice)
	, m_BufferAllocations(pDevice, bufferBlockSize)
	, m_ImageAllocations(pDevice, imageBlockSize)
{
	Initialize();
}

ResourceManager::~ResourceManager()
{
	Destroy();
}

ResourceManager::Allocation ResourceManager::AllocateBuffer(VkBuffer buffer, bool transient, VkMemoryPropertyFlags memoryProperty)
{
	VkMemoryRequirements memoryRequirements = {};
	vkGetBufferMemoryRequirements(GetRawDevice(), buffer, &memoryRequirements);
	uint32_t memoryTypeIndex = 0;
	bool foundMemory = GetDevice()->FindMemoryType(memoryRequirements.memoryTypeBits, memoryProperty, &memoryTypeIndex);
	K3D_ASSERT(foundMemory);
	ResourceManager::ResDesc<VkBuffer> objDesc = {};
	objDesc.Object = buffer;
	objDesc.IsTransient = transient;
	objDesc.MemoryTypeIndex = memoryTypeIndex;
	objDesc.MemoryProperty = memoryProperty;
	objDesc.MemoryRequirements = memoryRequirements;
	return m_BufferAllocations.Allocate(objDesc);
}

ResourceManager::Allocation ResourceManager::AllocateImage(VkImage image, bool transient, VkMemoryPropertyFlags memoryProperty)
{
	VkMemoryRequirements memoryRequirements = {};
	vkGetImageMemoryRequirements(GetRawDevice(), image, &memoryRequirements);
	uint32_t memoryTypeIndex = 0;
	bool foundMemory = GetDevice()->FindMemoryType(memoryRequirements.memoryTypeBits, memoryProperty, &memoryTypeIndex);
	K3D_ASSERT(foundMemory);
	ResourceManager::ResDesc<VkImage> objDesc = {};
	objDesc.Object = image;
	objDesc.IsTransient = transient;
	objDesc.MemoryTypeIndex = memoryTypeIndex;
	objDesc.MemoryProperty = memoryProperty;
	objDesc.MemoryRequirements = memoryRequirements;
	return  m_ImageAllocations.Allocate(objDesc);
}

void ResourceManager::Initialize()
{
}

void ResourceManager::Destroy()
{
	m_BufferAllocations.Destroy();
	m_ImageAllocations.Destroy();
}

K3D_VK_END