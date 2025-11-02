#include "pch.h"
#include "VulkanImageView.h"
#include "VulkanImage.h"
#include "Conversions.h"

VulkanImageView::VulkanImageView(vk::Device device, const VulkanImageViewDesc& desc)
	:m_Owning(desc.owning),m_Device(device),m_Desc(desc)
{
	if (desc.preMadeView)
	{
		m_View = desc.preMadeView;
	}
	else
	{
		VulkanImage* image = (VulkanImage*)desc.image;
		const ImageDesc& imageDesc = image->GetDesc();

		auto viewInfo = vk::ImageViewCreateInfo()
			.setImage(image->getVkImage())
			.setFormat(GetVkFormat(imageDesc.format))
			.setViewType(GetVkViewType(desc.type))
			.setSubresourceRange({ GetVkImageAspect(desc.aspect),0,1,desc.baseLayer,desc.layers });

		m_View = device.createImageView(viewInfo);
	}
}

VulkanImageView::~VulkanImageView()
{
	if (m_Owning)
		m_Device.destroyImageView(m_View);
}
