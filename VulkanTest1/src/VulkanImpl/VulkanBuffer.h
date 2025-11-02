#pragma once

#include "abstraction/Buffer.h"
#include <vulkan/vulkan.hpp>

struct VulkanBufferDesc : public BufferDesc
{
	vk::PhysicalDevice physicalDevice;
	std::vector<uint32_t> queueFamilies;
};

class VulkanBuffer : public Buffer
{
public:
	VulkanBuffer(vk::Device device, const VulkanBufferDesc& desc);
	~VulkanBuffer();

	virtual void* Map() override;
	virtual void UnMap() override;
public:
	inline vk::DeviceMemory getVkMemory() { return m_Memory; }
	inline vk::Buffer getVkBuffer() { return m_Buffer; }
	inline virtual const BufferDesc& GetDesc() const override { return m_Desc; }

private:
	vk::Device m_Device;
	vk::DeviceMemory m_Memory;
	vk::Buffer m_Buffer;

	BufferDesc m_Desc;

	bool m_DoFlush, m_DoInvalidate;
};