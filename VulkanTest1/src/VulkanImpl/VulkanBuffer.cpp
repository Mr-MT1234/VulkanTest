#include "pch.h"
#include "VulkanBuffer.h"
#include <tuple>

struct MemoryPreferences
{
	vk::MemoryPropertyFlags reqs = {}, prefs = {};
};
inline static MemoryPreferences GetVkMemoryFlags(ResourceAccessibilityBits cpuAccessibility, ResourceAccessRate gpuAccessRate)
{
	vk::MemoryPropertyFlags req = {}, pref = {};

	switch (gpuAccessRate)
	{
	case ResourceAccessRate::Rare:		
		break;
	case ResourceAccessRate::Frequent:	
		pref |= vk::MemoryPropertyFlagBits::eDeviceLocal;
		break;
	default:
		ASSERT(false, "Unknown gpuAccessRate (value = %u)",gpuAccessRate)
		break;
	}

	if (cpuAccessibility & ResourceAccessibilityBits::Read)
	{
		req |= vk::MemoryPropertyFlagBits::eHostVisible;
		pref |= vk::MemoryPropertyFlagBits::eHostCached;
	}
	if (cpuAccessibility & ResourceAccessibilityBits::Write)
	{
		req |= vk::MemoryPropertyFlagBits::eHostVisible;
	}
	if (cpuAccessibility & (ResourceAccessibilityBits::Read | ResourceAccessibilityBits::Write))
	{
		pref |= vk::MemoryPropertyFlagBits::eHostCoherent;
	}

	return { req , pref };
}

inline static vk::BufferUsageFlags GetVkUsage(BufferUsageFlags usage)
{
	constexpr vk::BufferUsageFlags null = {};

	return    ((usage & BufferUsageBits::TransferSrc)   ? vk::BufferUsageFlagBits::eTransferSrc   : null)
			| ((usage & BufferUsageBits::TransferDst)   ? vk::BufferUsageFlagBits::eTransferDst   : null)
			| ((usage & BufferUsageBits::VertexBuffer)  ? vk::BufferUsageFlagBits::eVertexBuffer  : null)
			| ((usage & BufferUsageBits::IndexBuffer)   ? vk::BufferUsageFlagBits::eIndexBuffer   : null)
			| ((usage & BufferUsageBits::StorageBuffer) ? vk::BufferUsageFlagBits::eStorageBuffer : null)
			| ((usage & BufferUsageBits::UniformBuffer) ? vk::BufferUsageFlagBits::eUniformBuffer : null);
}

inline static std::tuple<uint32_t,vk::MemoryPropertyFlags> FindMemoryIndex(const vk::PhysicalDevice device, const uint32_t choices, const MemoryPreferences& preferences)
{
	auto memoryProps = device.getMemoryProperties();

	auto RateMemory = [&](const vk::MemoryType& mem) -> float
	{
		auto countBits = [](uint32_t c)
		{
			c = c - ((c >> 1) & 0x55555555);
			c = (c & 0x33333333) + ((c >> 2) & 0x33333333);
			return ((c + (c >> 4) & 0xF0F0F0F) * 0x1010101) >> 24;
		};
		float s = 0;
		s += ((mem.propertyFlags & preferences.reqs) == preferences.reqs);
		s += countBits((uint32_t)(mem.propertyFlags & preferences.prefs)) * s * 0.1f;
		return s;
	};

	float bestScore = 0;
	uint32_t bestIdx = -1;

	for (uint32_t i = 0; i < memoryProps.memoryTypeCount; i++) 	if (choices & Bit(i))
	{
		float score = RateMemory(memoryProps.memoryTypes[i]);
		if (score > bestScore)
		{
			bestIdx = i;
			bestScore = score;
		}

	}

	ASSERT(bestScore > 0, "Could not find a suitable memory");
	return {bestIdx,memoryProps.memoryTypes[bestIdx].propertyFlags};
}

VulkanBuffer::VulkanBuffer(vk::Device device, const VulkanBufferDesc& desc)
{
	ASSERT(desc.size, "Inacceptable buffer size : %u", desc.size);
	ASSERT(BufferUsageFlags(desc.usage), "Inacceptable buffer usage : %u", desc.usage);


	auto bufferInfo = vk::BufferCreateInfo()
		.setSharingMode(desc.queueFamilies.size() == 1 ? vk::SharingMode::eExclusive : vk::SharingMode::eConcurrent)
		.setQueueFamilyIndexCount(desc.queueFamilies.size()).setPQueueFamilyIndices(desc.queueFamilies.data())
		.setSize(desc.size)
		.setUsage(GetVkUsage(desc.usage));

	m_Buffer = device.createBuffer(bufferInfo);

	vk::MemoryRequirements reqs = device.getBufferMemoryRequirements(m_Buffer);
	auto [memoryIdx, memoryProps] = FindMemoryIndex(desc.physicalDevice, reqs.memoryTypeBits, GetVkMemoryFlags(desc.cpuAccessibility, desc.gpuAccessRate));

	auto allocInfo = vk::MemoryAllocateInfo().setAllocationSize(reqs.size)
		.setMemoryTypeIndex(memoryIdx);

	m_Memory = device.allocateMemory(allocInfo);

	device.bindBufferMemory(m_Buffer, m_Memory, 0);

	m_Desc = { desc };
	m_Device = device;

	if (memoryProps & vk::MemoryPropertyFlagBits::eHostCoherent)
	{
		m_DoFlush = false;
		m_DoInvalidate = false;
	}
	else if (desc.cpuAccessibility & ResourceAccessibilityBits::Read)
	{
		m_DoFlush = false;
		m_DoInvalidate = true;
	}
	else if (desc.cpuAccessibility & ResourceAccessibilityBits::Write)
	{
		m_DoFlush = true;
		m_DoInvalidate = false;
	}
}

VulkanBuffer::~VulkanBuffer()
{
	m_Device.destroyBuffer(m_Buffer);
	m_Device.freeMemory(m_Memory);
}

void* VulkanBuffer::Map()
{
	vk::MappedMemoryRange mmr = { m_Memory, 0, m_Desc.size };
	void* ptr = m_Device.mapMemory(mmr.memory, mmr.offset, mmr.size, {});

	if (m_DoInvalidate)
		m_Device.invalidateMappedMemoryRanges(mmr);

	return ptr;
}

void VulkanBuffer::UnMap()
{
	vk::MappedMemoryRange mmr = { m_Memory, 0, m_Desc.size };

	if(m_DoFlush)
		m_Device.flushMappedMemoryRanges(mmr);

	m_Device.unmapMemory(m_Memory);
}
