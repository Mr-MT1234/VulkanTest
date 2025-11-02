#pragma once

#include "abstraction/Receipe.h"
#include <vulkan/vulkan.hpp>

struct VulkanReceipeDesc
{
	vk::Semaphore semaphore;
	bool ownSemaphore = true;
	vk::Fence fence;
	bool ownFence = true;
};

class VulkanReceipe : public Receipe
{
public:
	VulkanReceipe(vk::Device d,const VulkanReceipeDesc& desc)
		:m_Device(d),m_Desc(desc)
	{}
	~VulkanReceipe()
	{
		if(m_Desc.ownFence)		m_Device.destroyFence(m_Desc.fence);
		if(m_Desc.ownSemaphore)	m_Device.destroySemaphore(m_Desc.semaphore);
	}

	inline vk::Fence getVkFence() const { return m_Desc.fence; }
	inline vk::Semaphore getVkSemaphore() const { return m_Desc.semaphore; }
private:
	vk::Device m_Device;
	VulkanReceipeDesc m_Desc;
};