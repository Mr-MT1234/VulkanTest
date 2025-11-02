#pragma once

#include "abstraction/Image.h"
#include <vulkan/vulkan.hpp>

struct VulkanImageDesc : ImageDesc
{
	std::set<uint32_t> queueFamilies;
	vk::Image preMadeHandle = nullptr;
	bool owning = true;
};

class VulkanImage : public Image
{
public:
	VulkanImage(vk::Device device, const VulkanImageDesc& desc);
	~VulkanImage();
public:

	inline virtual const ImageDesc& GetDesc() override { return m_Desc; }

public:
	inline vk::Image getVkImage() { return m_Image; }
	inline bool isOwning() { return m_Owning; }
private:
	vk::Device m_Device;
	vk::Image m_Image;
	ImageDesc m_Desc;

	bool m_Owning;
};
