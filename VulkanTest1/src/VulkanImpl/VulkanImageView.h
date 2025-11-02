#pragma once

#include "abstraction/ImageView.h"
#include <vulkan/vulkan.hpp>

struct VulkanImageViewDesc : public ImageViewDesc
{
	vk::ImageView preMadeView = nullptr;
	bool owning = true;
};

class VulkanImageView : public ImageView
{
public:
	VulkanImageView(vk::Device device, const VulkanImageViewDesc& desc);
	~VulkanImageView();

public:
	inline const vk::ImageView& getVkView() { return m_View; }
private:
	ImageViewDesc m_Desc;
	bool m_Owning;

	vk::ImageView m_View;
	vk::Device m_Device;
};