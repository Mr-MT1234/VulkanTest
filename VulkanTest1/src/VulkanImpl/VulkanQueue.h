#pragma once

#include "abstraction/Queue.h"
#include <vulkan/vulkan.hpp>

class VulkanQueue : public Queue
{
public:
	VulkanQueue(vk::Queue queue) : m_Queue(queue){}
	inline vk::Queue getVkQueue() { return m_Queue; }
private:
	vk::Queue m_Queue;
};