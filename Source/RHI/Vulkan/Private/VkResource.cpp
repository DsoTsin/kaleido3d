#include "VkCommon.h"
#include "Public/VkRHI.h"
#include "VkUtils.h"
#include "VkEnums.h"
#include <algorithm>

K3D_VK_BEGIN

Resource::Ptr Resource::Map(uint64 offset, uint64 size)
{
	Resource::Ptr ptr;
	K3D_VK_VERIFY(vkMapMemory(GetRawDevice(), m_DeviceMem, m_AllocationOffset+offset, size, 0, &ptr));
	return ptr;
}

Resource::~Resource()
{
	VKLOG(Info, "Resource-Destroying Resource..");
	//vkFreeMemory(GetRawDevice(), m_DeviceMem, nullptr);
}

Buffer::Buffer(Device::Ptr pDevice, rhi::ResourceDesc const &desc)
: Resource(pDevice)
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
	VKLOG(Info, "Buffer-Destroying Resource..");
	if (VK_NULL_HANDLE != m_BufferView)
	{
		vkDestroyBufferView(GetRawDevice(), m_BufferView, nullptr);
		m_BufferView = VK_NULL_HANDLE;
	}
	if (VK_NULL_HANDLE != m_Buffer)
	{
		vkDestroyBuffer(GetRawDevice(), m_Buffer, nullptr);
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
	K3D_VK_VERIFY(vkCreateBuffer(GetRawDevice(), &createInfo, nullptr, &m_Buffer));
	ResourceManager::Allocation alloc = GetDevice()->GetMemoryManager()->AllocateBuffer(m_Buffer, false, m_MemoryBits);
	K3D_ASSERT(VK_NULL_HANDLE != alloc.Memory);

	m_DeviceMem = alloc.Memory;
	m_AllocationOffset = alloc.Offset;
	m_AllocationSize = alloc.Size;
	m_Size = size;

	VKLOG(Info, "Buffer reqSize:(%d) allocated:(%d) offset:(%d) address:(0x%0x).",
		  m_Size, m_AllocationSize, m_AllocationOffset, m_DeviceMem);

	m_BufferInfo.buffer = m_Buffer;
	m_BufferInfo.offset = 0;
	m_BufferInfo.range = m_Size;
	
	K3D_VK_VERIFY(vkBindBufferMemory(GetRawDevice(), m_Buffer, m_DeviceMem, m_AllocationOffset));
}

StageBuffer::StageBuffer(Device::Ptr pDevice, rhi::ResourceDesc const & desc)
	: Buffer(pDevice, desc)
{
}

StageBuffer::~StageBuffer()
{
}

Texture::Texture(Device::Ptr pDevice, rhi::TextureDesc const &desc)
: Resource(pDevice)
{
	Create(desc);
}

void Texture::Create(rhi::TextureDesc const & desc)
{
	m_ImageInfo = ImageInfo::FromRHI(desc);
	K3D_VK_VERIFY(vkCreateImage(GetRawDevice(), &m_ImageInfo, nullptr, &m_Image));
	m_ImageViewInfo = ImageViewInfo::From(m_ImageInfo, m_Image);
	K3D_VK_VERIFY(vkCreateImageView(GetRawDevice(), &m_ImageViewInfo, nullptr, &m_ImageView));

	ResourceManager::Allocation alloc = GetDevice()->GetMemoryManager()->AllocateImage(m_Image, false, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
	K3D_ASSERT(VK_NULL_HANDLE != alloc.Memory);

	m_DeviceMem = alloc.Memory;
	m_AllocationOffset = alloc.Offset;
	m_AllocationSize = alloc.Size;

	K3D_VK_VERIFY(vkBindImageMemory(GetRawDevice(), m_Image, m_DeviceMem, m_AllocationOffset));
}

Texture::Texture(Device::Ptr pDevice, rhi::ResourceDesc const & Desc)
	: Resource(pDevice)
{
	Create(Desc.TextureDesc);
}

Texture::~Texture()
{
	if (GetRawDevice() == VK_NULL_HANDLE)
		return;
	if (VK_NULL_HANDLE != m_ImageView)
	{
		vkDestroyImageView(GetRawDevice(), m_ImageView, nullptr);
		m_ImageView = VK_NULL_HANDLE;
	}
	if (VK_NULL_HANDLE != m_Image && m_SelfOwn)
	{
		vkDestroyImage(GetRawDevice(), m_Image, nullptr);
		m_Image = VK_NULL_HANDLE;
	}
	VKLOG(Info, "Texture-Destroyed..");
}

SpTexture Texture::CreateFromSwapChain(VkImage image, VkImageView view, VkImageViewCreateInfo info, Device::Ptr pDevice)
{
	return std::make_shared<Texture>(image, view, info, pDevice, false);
}

SpTexture Texture::CreateRenderTexture(Device::Ptr pDevice, rhi::TextureDesc const & desc)
{
	return SpTexture(new Texture(pDevice, desc));
}

void Texture::Create()
{
	VkImageCreateInfo imageCreateInfo = {};
	/*imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.pNext = nullptr;
	imageCreateInfo.imageType = mImageType;
	imageCreateInfo.format = mFormat.mInternalFormat;
	imageCreateInfo.extent = mExtent;
	imageCreateInfo.mipLevels = mFormat.mMipLevels;
	imageCreateInfo.arrayLayers = mFormat.mArrayLayers;
	imageCreateInfo.samples = mFormat.mSamples;
	imageCreateInfo.tiling = mFormat.mTiling;
	imageCreateInfo.usage = mFormat.mUsage;
	imageCreateInfo.flags = mFormat.mCreateFlags;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCreateInfo.queueFamilyIndexCount = 0;
	imageCreateInfo.pQueueFamilyIndices = nullptr;
	imageCreateInfo.initialLayout = mFormat.mInitialLayout;*/
	K3D_VK_VERIFY(vkCreateImage(GetRawDevice(), &imageCreateInfo, nullptr, &m_Image));
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
	VKLOG(Info,"%s alloca size=%ld", __K3D_FUNC__, poolSize);
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
	VKLOG(Info,"%s alignedOffset=%ld", __K3D_FUNC__, alignedOffst);
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
	::Concurrency::Mutex::AutoLock lock(&m_Mutex);
	for (auto& pool : m_Pools) {
		vkFreeMemory(GetRawDevice(), pool->m_Memory, nullptr);
	}
	m_Pools.clear();
}

template<typename VkObjectT>
ResourceManager::Allocation
ResourceManager::PoolManager<VkObjectT>::Allocate(const typename ResourceManager::ResDesc<VkObjectT>& objDesc)
{
	::Concurrency::Mutex::AutoLock lock(&m_Mutex);
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