#include "pch.h"
#include "VulkanSwapchain.h"
#include "VulkanImpl/VulkanImageView.h"
#include "VulkanImpl/VulkanImage.h"
#include "VulkanImpl/VulkanReceipe.h"
#include <GLFW/glfw3.h>
#include "Conversions.h"

VulkanSwapchain::VulkanSwapchain(vk::Device device, const VulkanSwapchainDesc& desc)
    : m_Device(device),m_PresentationQueue(desc.presentationQueue),m_CurrentIndex(-1)
{
    ASSERT(desc.surfaceDetails.avlFormats.size(), "No Format is Suported");
    ASSERT(desc.surfaceDetails.avlPresentModes.size(), "No present mode is Suported");

    vk::Format format;
    vk::ColorSpaceKHR colorSpace;
    {
        bool found = false;
        for (auto& surfaceFormat : desc.surfaceDetails.avlFormats)
        {
            if (surfaceFormat.format == vk::Format::eR8G8B8A8Srgb && surfaceFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
            {
                format = surfaceFormat.format;
                colorSpace = surfaceFormat.colorSpace;
                found = true;
                break;
            }
        }
        if (!found)
        {
            format = desc.surfaceDetails.avlFormats[0].format;
            colorSpace = desc.surfaceDetails.avlFormats[0].colorSpace;
        }
    }

    vk::PresentModeKHR presentMode;
    {

        uint32_t i = 0;
        if (desc.presentMode == PresentMode::VSync)
        {
            presentMode = vk::PresentModeKHR::eFifo;
        }
        else
        {
            float bestScore = 0;
            uint32_t bestIdx = -1;

            for (auto& mode : desc.surfaceDetails.avlPresentModes)
            {
                float score = 0;


                if (mode == vk::PresentModeKHR::eMailbox) score += 1000;
                else if (mode == vk::PresentModeKHR::eImmediate) score += 100;

                if (score > bestScore)
                {
                    bestScore = score;
                    bestIdx = i;
                }

                i++;
            }

            ASSERT(bestScore > 0, "No present Mode is support");
            presentMode = desc.surfaceDetails.avlPresentModes[bestIdx];
        }

    }

    uint32_t minImageCount;
    {
        minImageCount = desc.surfaceDetails.capabilities.minImageCount + 1;
        if (desc.surfaceDetails.capabilities.maxImageCount != 0)
            minImageCount = std::min(minImageCount, desc.surfaceDetails.capabilities.maxImageCount);
    }

    vk::Extent2D extend;
    {
        auto& [w, h] = desc.imagesDimensions;

        ASSERT((desc.surfaceDetails.capabilities.currentExtent.width  == UINT32_MAX) || (w >= desc.surfaceDetails.capabilities.minImageExtent.width && w <= desc.surfaceDetails.capabilities.maxImageExtent.width),"Incompatible Swapchain Image size");
        ASSERT((desc.surfaceDetails.capabilities.currentExtent.height == UINT32_MAX) || (h >= desc.surfaceDetails.capabilities.minImageExtent.height && h <= desc.surfaceDetails.capabilities.maxImageExtent.height), "Incompatible Swapchain Image size");

        extend = { w,h };
    }

    vk::SharingMode sharingMode;
    std::vector<uint32_t> queues(desc.queueFamilies.begin(), desc.queueFamilies.end());
    {
        sharingMode = queues.size() == 1 ? vk::SharingMode::eExclusive : vk::SharingMode::eConcurrent;
    }

    vk::SwapchainCreateInfoKHR swapcahainInfo = { {},
        desc.surfaceDetails.surface,
        minImageCount,
        format,colorSpace,
        extend,
        1,
        vk::ImageUsageFlagBits::eColorAttachment,
        sharingMode,(uint32_t)queues.size(),queues.data(),
        vk::SurfaceTransformFlagBitsKHR::eIdentity,
        vk::CompositeAlphaFlagBitsKHR::eOpaque,
        presentMode,
        true
    };

    m_Swapchain = device.createSwapchainKHR(swapcahainInfo);

    m_ImagesDesc.format = GetFormat(format);
    m_ImagesDesc.dimensions = { extend.width,extend.height,1 };
    m_ImagesDesc.type = ImageType::e2D;
    m_ImagesDesc.layers = 1;

    auto images = device.getSwapchainImagesKHR(m_Swapchain);
    m_Images.resize(images.size());
    m_Views.resize(images.size());

    VulkanImageDesc imageDesc;
    {
        imageDesc.dimensions = { desc.imagesDimensions.width,desc.imagesDimensions.height,1 };
        imageDesc.format = GetFormat(format);
        imageDesc.layers = 1;
        imageDesc.owning = false;
        imageDesc.queueFamilies = desc.queueFamilies;
        imageDesc.type = ImageType::e2D;
    }
    for (uint32_t i = 0; i < m_Images.size(); i++)
    {
        imageDesc.preMadeHandle = images[i];
        m_Images[i] = new VulkanImage(device, imageDesc);
    }

    VulkanImageViewDesc viewDesc;
    {
        viewDesc.baseLayer = 0;
        viewDesc.layers = 1;
        viewDesc.dimensions = m_ImagesDesc.dimensions;
        viewDesc.type = ImageViewType::e2D;
        viewDesc.aspect = ImageViewAspect::Color;
    }
    for (uint32_t i = 0; i < m_Views.size(); i++)
    {
        viewDesc.image = m_Images[i];
        m_Views[i] = new VulkanImageView(device, viewDesc);
    }

    m_Fence = device.createFence({});
}

/*VulkanSwapchain::VulkanSwapchain(const SwapchainDesc& desc,VulkanSurfaceDetails,std::set<uint32_t> queueFamilies)
{
    ASSERT(details.formats.size(), "No Format is Suported");
    ASSERT(details.presentModes.size(), "No present mode is Suported");

    vk::Format format;
    vk::ColorSpaceKHR colorSpace;
    {
        bool found = false;
        for (auto& surfaceFormat : details.formats)
        {
            if (surfaceFormat.format == vk::Format::eR8G8B8A8Srgb && surfaceFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
            {
                format = surfaceFormat.format;
                colorSpace = surfaceFormat.colorSpace;
                found = true;
                break;
            }
        }
        if (!found)
        {
            format = details.formats[0].format;
            colorSpace = details.formats[0].colorSpace;
        }
    }

    vk::PresentModeKHR presentMode;
    {

        uint32_t i = 0;
        if (desc.presentMode == PresentMode::VSync)
        {
            presentMode = vk::PresentModeKHR::eFifo;
        }
        else
        {
            float bestScore = 0;
            uint32_t bestIdx = -1;

            for (auto& mode : details.presentModes)
            {
                float score = 0;


                if (mode == vk::PresentModeKHR::eMailbox) score += 1000;
                else if (mode == vk::PresentModeKHR::eImmediate) score += 100;

                if (score > bestScore)
                {
                    bestScore = score;
                    bestIdx = i;
                }

                i++;
            }

            ASSERT(bestScore > 0, "No present Mode is support");
            presentMode = details.presentModes[bestIdx];
        }
        
    }

    uint32_t minImageCount;
    {
        minImageCount = details.capabilities.minImageCount + 1;
        if (details.capabilities.maxImageCount != 0)
            minImageCount = std::min(minImageCount, details.capabilities.maxImageCount);
    }

    vk::Extent2D extend;
    {
        if (details.capabilities.currentExtent == vk::Extent2D(-1, -1))
        {
            int w, h;
            glfwGetWindowSize(window,&w,&h);
            uint32_t width = w, height = h;
            extend.width = std::clamp(width, details.capabilities.minImageExtent.width, details.capabilities.maxImageExtent.width);
            extend.height = std::clamp(height, details.capabilities.minImageExtent.height, details.capabilities.maxImageExtent.height);
        }
        else
        {
            extend = details.capabilities.currentExtent;
        }
    }

    vk::SharingMode sharingMode;
    std::vector<uint32_t> queues(queueFamilies.begin(),queueFamilies.end());
    {
        sharingMode = queues.size() == 1 ? vk::SharingMode::eExclusive : vk::SharingMode::eConcurrent;
    }

    vk::SwapchainCreateInfoKHR swapcahainInfo = { {},
        surface,
        minImageCount,
        format,colorSpace,
        extend,
        1,
        vk::ImageUsageFlagBits::eColorAttachment,
        sharingMode,(uint32_t)queues.size(),queues.data(),
        vk::SurfaceTransformFlagBitsKHR::eIdentity,
        vk::CompositeAlphaFlagBitsKHR::eOpaque,
        presentMode,
        true
    };

    m_Swapchain = device.createSwapchainKHR(swapcahainInfo);

    m_ImagesDesc.format = GetFormat(format);
    m_ImagesDesc.dimensions = { extend.width,extend.height };
    m_ImagesDesc.type = ImageType::e2D;
    m_ImagesDesc.layers = 1;

    auto images = device.getSwapchainImagesKHR(m_Swapchain);
    m_Views.resize(images.size());
    //for (uint32_t i = 0; i < swapchain.views.size(); i++)
    //{
    //    swapchain.views[i] = createImageView(swapchain.images[i], vk::ImageViewType::e2D, swapchain.format);
    //}
}
*/
VulkanSwapchain::~VulkanSwapchain()
{
    for (auto& view : m_Views) delete view;
    m_Device.destroySwapchainKHR(m_Swapchain);
    m_Device.destroyFence(m_Fence);
}

SwapchainImage VulkanSwapchain::GetNextImage()
{
    m_Device.resetFences(m_Fence);
    m_CurrentIndex = m_Device.acquireNextImageKHR(m_Swapchain, UINT32_MAX, nullptr, m_Fence).value;
    m_Device.waitForFences(m_Fence, true, UINT32_MAX);

    return {m_Views[m_CurrentIndex],m_CurrentIndex };
}

void VulkanSwapchain::Present(ArrayProxy<Receipe*> receipes)
{
    std::vector<vk::Semaphore> semaphores(receipes.size());
    uint32_t i = 0;
    for (const auto& receipe : receipes)
        semaphores[i++] = static_cast<VulkanReceipe*>(receipe)->getVkSemaphore();


    auto presentInfo = vk::PresentInfoKHR()
        .setSwapchainCount(1).setPSwapchains(&m_Swapchain)
        .setWaitSemaphoreCount(semaphores.size()).setPWaitSemaphores(semaphores.data())
        .setPImageIndices(&m_CurrentIndex);
    
    m_PresentationQueue.presentKHR(presentInfo);
}


