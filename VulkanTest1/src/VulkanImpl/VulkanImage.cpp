#include "pch.h"
#include "VulkanImage.h"
#include "Conversions.h"

VulkanImage::VulkanImage(vk::Device device, const VulkanImageDesc& desc)
	:m_Device(device),m_Owning(desc.owning)
{
    if (desc.preMadeHandle)
    {
        m_Image = desc.preMadeHandle;
        m_Desc = desc;
    }
    else
    {
        ASSERT(false, "Image creation not supported yet");
        vk::ImageCreateFlags flags = {};

        if (desc.type == ImageType::e2D && desc.layers >= 6) flags |= vk::ImageCreateFlagBits::eCubeCompatible;
        else if (desc.type == ImageType::e3D) flags |= vk::ImageCreateFlagBits::e2DArrayCompatible;

        auto imageinfo = vk::ImageCreateInfo()
            .setArrayLayers(desc.layers)
            .setExtent({ desc.dimensions.width,desc.dimensions.height ,desc.dimensions.depth })
            .setFlags(flags)
            .setFormat(GetVkFormat(desc.format))
            .setImageType(GetVkType(desc.type));
    }
}

VulkanImage::~VulkanImage()
{
    if (m_Owning && m_Image)
        m_Device.destroyImage(m_Image);
}
